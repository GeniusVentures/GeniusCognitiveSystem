/**
 * @file       gcs_core_ffi.cpp
 * @brief      gcs_ffi thunk — parses GcsCommand publishes and pushes GcsEvent bytes.
 * @details    Implements the four-function topic pub/sub C ABI declared in
 *             gcs_core.h (D-27/D-29). Dart publishes serialized gcs.chat.GcsCommand
 *             envelopes (oneof join_topic/send_text) to the command topic; this
 *             thunk parses them, dispatches through gcs::CoreSession, stamps the
 *             authoritative ChatMessageState fields (D-04 — C++ owns state), and
 *             pushes serialized gcs.chat.GcsEvent envelopes (message/room-list/
 *             readiness/raw-error-string) toward the registered Dart NativePort
 *             as Dart_CObject typed-data (uint8) posted through the vendored
 *             Dart API_DL indirection (Dart_PostCObject_DL). Thread-safe via
 *             global mutex; no exceptions escape the ABI; caller-owned payload
 *             buffers are copied inside the call and never retained.
 * @date       2026-08-26
 * @copyright  (c) 2026 GNUS.AI
 */
#include "gcs_core.h"
#include "dart_api_dl.h"
#include "proto/gcs_chat.pb.h"

#include "lib/gcs_core.hpp"
#include "lib/gcs_messaging.hpp"
#include "lib/gcs_crypto.hpp"
#include "lib/gcs_entity_store.hpp"
#include "gcs_storage/common/logging.hpp"

#include "GeniusSDK.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <system_error>
#include <vector>

namespace
{
    // Dart -> C++ ingress topic (D-27: commands are topic publishes to one topic).
    constexpr const char* kCommandTopic = "gcs/command";
    // Pre-joined smoke topics (D-26 requires >= 2) so the pushed RoomList is non-empty.
    constexpr const char* kSmokeTopicA = "gcs/chat/smoke-test";
    constexpr const char* kSmokeTopicB = "gcs/chat/smoke-test-2";
    // Maximum entity display-name length in Unicode code points (T-02-09 —
    // matches the Dart dialog's kMaxNameLength, which counts characters, not
    // bytes; the FFI re-validates so any client, not just the dialog, is
    // bounded). Counted via Utf8CodePointCount so non-ASCII names keep the
    // same effective cap on both sides of the boundary (WR-05).
    constexpr size_t kMaxEntityNameLength = 64;
    // Maximum topic-string length in bytes across the FFI command arms
    // (join_topic/send_text room_topic; IN-08 hardening). Topics are ASCII by
    // construction — derived topics are "gcs/chat/" + entity id — so a byte
    // cap is exact. Sized so the longest minted derived topic always fits:
    // "gcs/chat/room-<13-digit ms>-<random-token>-<seq>" stays under ~60
    // bytes, leaving 2x headroom.
    constexpr size_t kMaxTopicLength = 128;
    // Room-topic namespace every GCS-minted topic lives under: smoke topics
    // ("gcs/chat/smoke-test") and derived room topics ("gcs/chat/room-...").
    // WR-03: join/send topics are validated against this namespace so the
    // archive key grammar stays unambiguous (see IsValidRoomTopicShape).
    constexpr const char kRoomTopicNamespace[] = "gcs/chat/";
    // Maximum message-text length in bytes at the FFI boundary (IN-08
    // hardening). A transport-size bound, not a character contract: no Dart
    // cap exists yet, so bytes bound the copied/serialized/persisted payload
    // directly — 4 KiB is generous for chat text while blocking the
    // multi-megabyte publishes the payload-narrowing guard alone permitted.
    constexpr size_t kMaxMessageTextLength = 4096;
    // Bounded in-flight drain window on the process-exit teardown path
    // (WR-01): the exit hook may not block forever on a wedged sender, so
    // the drain gives up after this timeout and proceeds best-effort.
    constexpr std::chrono::seconds kExitTeardownDrainTimeout{ 2 };
    // Dev config accepted by GeniusSDKInit's parser — offline-safe placeholder
    // token parameters (identical to the C++ test fixtures' kDevConfig). Used
    // when gcs_init boots the embedded node itself.
    constexpr const char kDevConfig[] = R"(
     {
       "Address": "0x0000000000000000000000000000000000000001",
       "Cut": "100",
       "TokenValue": "1000",
       "TokenID": "0x0000000000000000000000000000000000000000000000000000000000000001"
     }
    )";

    std::mutex g_mutex;                            // guards g_session + g_entities + g_messaging + topic sets
    std::unique_ptr<gcs::CoreSession> g_session;   // Phase 1: single global session
    std::unique_ptr<gcs::EntityStore> g_entities;  // Phase 2: entity catalog over g_session
    std::unique_ptr<gcs::Messaging> g_messaging;   // Phase 3: messaging over g_session (D-03/D-08)
    std::vector<std::string> g_roomTopics;         // joined topic set (guarded by g_mutex)
    std::vector<std::string> g_derivedTopics;      // autoJoin-derived subset of g_roomTopics (D-04)
    std::vector<std::string> g_explicitTopics;     // join_topic-joined subset (WR-03: survives derived
                                                   // eviction; guarded by g_mutex)
    bool g_sdkBootedHere = false;                  // gcs_init booted the embedded GeniusSDK node —
                                                   // pairs that boot with GeniusSDKShutdown in
                                                   // TeardownSessionAndNode (guarded by g_mutex)
    bool g_exitHookRegistered = false;             // std::atexit pairing hook registered once per
                                                   // process (written only under g_mutex)
    std::atomic<int64_t> g_dartPort{ 0 };          // registered Dart port (0 = unregistered)
    // One-shot guard for the per-process Dart API_DL table state check in gcs_init
    // (the table itself is initialized via the exported Dart_InitializeApiDL).
    std::atomic<bool> g_apiDlInitialized{ false };
    // In-flight count of operations running UNLOCKED against the session/
    // messaging globals (send_text's SendMessage window, live subscribes,
    // receive funnels) and the condition variable gcs_shutdown waits on for
    // the count to return to zero before teardown destroys those objects
    // (WR-02 — the raw-pointer unlocked windows are safe because teardown
    // provably drains them; the "Dart serializes commands and shutdown"
    // assumption is enforced, not assumed). Both guarded by g_mutex.
    unsigned int g_inFlight = 0;
    std::condition_variable g_idleCond;
    // Teardown intake gate (CR-02): receive callbacks (the pubsub live bridge
    // and the CRDT DagWorker bridge) check this ATOMICALLY before taking
    // g_mutex, so a shutdown flips it and no callback funnels into a session
    // mid-teardown. Re-armed (cleared) by the next gcs_init.
    std::atomic<bool> g_receivingDisabled{ false };
    // Teardown's unlocked destruction phase is in progress (CR-02): gcs_init
    // refuses to build a session while the old one is destroyed with g_mutex
    // released — the mutex no longer excludes init across the teardown joins.
    std::atomic<bool> g_tearingDown{ false };

    /**
     * \brief Counts Unicode code points in a UTF-8 string.
     *
     * A code point begins at every byte that is not a 0b10xxxxxx continuation
     * byte, so the count of non-continuation bytes equals the code-point
     * count. Mirrors the Dart dialog's character-based name cap: counting
     * raw bytes instead would shrink the effective limit for every non-ASCII
     * name (30 CJK characters are 90 UTF-8 bytes) and reject names the
     * client already accepted (WR-05).
     *
     * \param[in] text  The UTF-8 string to count.
     * \return The number of Unicode code points in text.
     */
    size_t Utf8CodePointCount( const std::string &text )
    {
        return static_cast<size_t>( std::count_if( text.begin(), text.end(),
            []( unsigned char byte ) { return ( byte & 0xC0 ) != 0x80; } ) );
    }

    /**
     * \brief Whether a room topic fits the unambiguous archive-key grammar (WR-03).
     *
     * Archive keys are "gcs/messages/<topic>/<id>" and RoomTopicFromKey
     * recovers the topic by dropping only the LAST '/' segment (the id), so
     * the grammar is ambiguous whenever one topic can be a slash-prefix of
     * another: the prefix scan for room "gcs/chat/a" also returns every
     * record of room "gcs/chat/a/b" — a cross-room data leak in plaintext
     * mode and spurious decrypt failures in encrypted mode. Every topic
     * this system mints is "gcs/chat/<single-segment-id>" (smoke topics,
     * derived room topics), so accepting only that shape at the FFI
     * boundary matches the real key space and makes no topic a
     * slash-prefix of any other.
     *
     * \param[in] roomTopic  The candidate room topic.
     * \return true when the topic is "gcs/chat/<id>" with a non-empty id and
     *         no further '/'.
     */
    bool IsValidRoomTopicShape( const std::string &roomTopic )
    {
        const size_t namespaceLength = std::char_traits<char>::length( kRoomTopicNamespace );
        return roomTopic.size() > namespaceLength
               && roomTopic.compare( 0, namespaceLength, kRoomTopicNamespace ) == 0
               && roomTopic.find( '/', namespaceLength ) == std::string::npos;
    }

    /**
     * \brief Derives the session base path everything GCS writes calls home.
     *
     * The node, the wallet, and the GCS log file all live beside the store:
     * for a db_path of "<base>/db" that is "<base>" (mirroring the C++ test
     * fixtures' temp-root + "/db" layout). A bare filename has no parent —
     * use the CWD. An empty db_path (store default) still needs a home for
     * the node and logs — the system temp dir keeps it out of the CWD.
     *
     * Per-instance contract: the base path IS the node's identity scope —
     * the wallet persists under it (own peer id and sender address per
     * base), and InitNetwork derives the libp2p port from the wallet
     * address (GenerateRandomPort(port_seed, address)). Host apps running
     * multiple instances must therefore give each one its own base path
     * (the Flutter app: data/KEY/db, so data/KEY is the base).
     *
     * \param[in] config The parsed GcsConfig carrying db_path.
     * \return The base directory for node data, wallet, and logs.
     */
    std::filesystem::path SessionBasePath( const gcs::chat::GcsConfig& config )
    {
        if ( config.db_path().empty() )
        {
            std::error_code tempEc;
            return std::filesystem::temp_directory_path( tempEc ) / "gcs";
        }
        std::filesystem::path basePath = std::filesystem::path( config.db_path() ).parent_path();
        if ( basePath.empty() )
        {
            basePath = ".";
        }
        return basePath;
    }

    /**
     * \brief Tears the global session and the embedded node down.
     *
     * Single teardown body shared by gcs_shutdown (passing its held
     * unique_lock) and the exit-time pairing hook (passing nullptr — see
     * EnsureSdkBooted). Port first so nothing posts into a dying Dart VM,
     * then the session, then the node — but only the node this library
     * booted: an externally booted node (host harness) outlives the session
     * and is not ours to tear down. Safe to call repeatedly: every arm
     * checks its own guard.
     *
     * When lock is non-null it must be HELD on entry: the guarded globals
     * are evicted under it, then it is RELEASED across the blocking
     * teardown calls and re-acquired before returning (CR-02). Every join
     * below can wait on a thread whose callback funnels through g_mutex —
     * GeniusSDKShutdown's node/pubsub threads (the EventSink takes
     * g_mutex), Messaging's archive worker (its Puts can trigger DagWorker
     * bridges that take g_mutex), and ShutdownNow's WaitForWorkersToExit
     * (the CRDT new-element bridge takes g_mutex). Holding g_mutex across
     * those joins is the CR-02 shutdown deadlock; releasing it lets the
     * gated callbacks acquire the mutex, observe the evicted (null)
     * globals, and unwind.
     *
     * The process-exit caller passes nullptr and holds no lock. WR-01: it
     * still gates receive intake first (atomically, before touching
     * anything), then acquires g_mutex best-effort — try_lock, because a
     * thread may have died holding it — so the eviction below is serialized
     * against receive callbacks and a concurrent gcs_shutdown, and the
     * in-flight count is drained under it with a BOUNDED wait (exit must not
     * hang on a wedged sender). When the try_lock fails the hook proceeds
     * destructively as before, with the intake gate already closed.
     *
     * \param[in] lock  gcs_shutdown's held unique_lock, or nullptr on the
     *                  process-exit path.
     */
    void TeardownSessionAndNode( std::unique_lock<std::mutex> *lock )
    {
        // WR-01: gate receive intake on EVERY path into this body — the exit
        // hook previously skipped this, so pubsub/DagWorker bridge callbacks
        // kept funnelling into g_messaging while the hook destroyed it.
        g_receivingDisabled.store( true );
        g_tearingDown.store( true );
        // Quiesce the node BEFORE tearing down the session. GcsGlobalDb
        // borrows the node's pubsub and shared graphsync Network (D-17), so
        // the session's ShutdownNow() destructor chain unsubscribes from the
        // node's pubsub and closes node-owned peer streams. Running that
        // chain while the node's threads are still live (pubsub reactor,
        // io threads, consensus timer) races destruction against concurrent
        // callbacks — a destroyed mutex gets locked and the noexcept
        // ~GraphsyncImpl terminates the process (EINVAL system_error —
        // SIGABRT on macOS app quit, crash report 2026-09-19 19:11).
        // GeniusSDKShutdown() runs ~GeniusNode's coordinated stop first; the
        // borrowed objects stay alive for the session teardown because the
        // shared Network owns shared_ptrs to the host and scheduler.
        const bool sdkBootedHere = g_sdkBootedHere;
        g_sdkBootedHere = false;

        // WR-01: the process-exit caller holds no lock. Acquire g_mutex
        // best-effort (try_lock — a thread may have died holding it, and the
        // hook must not wedge at exit) so the eviction below is serialized
        // against receive callbacks and a concurrent gcs_shutdown, then
        // drain the in-flight count under it with a bounded wait. Never
        // attempted when the caller already holds the lock (double-locking a
        // std::mutex on the normal path would be undefined).
        std::unique_lock<std::mutex> exitLock( g_mutex, std::defer_lock );
        if ( lock == nullptr )
        {
            exitLock.try_lock();
            if ( exitLock.owns_lock() )
            {
                g_idleCond.wait_for( exitLock, kExitTeardownDrainTimeout,
                                     [] { return g_inFlight == 0; } );
            }
            else
            {
                spdlog::warn( "gcs_ffi: exit-path teardown could not acquire the session "
                              "mutex — proceeding without the in-flight drain" );
            }
        }

        // Evict the guarded globals while the lock is held, then destroy the
        // evicted objects with the lock RELEASED. The eviction is what makes
        // the unlocked phase safe: any late callback or command that
        // acquires g_mutex afterwards sees null globals and no-ops instead
        // of touching half-destroyed state.
        std::unique_ptr<gcs::CoreSession> session;
        std::unique_ptr<gcs::EntityStore> entities;
        std::unique_ptr<gcs::Messaging> messaging;
        if ( g_session != nullptr )
        {
            g_dartPort = 0; // unregister the port BEFORE teardown
            session   = std::move( g_session );
            entities  = std::move( g_entities );
            messaging = std::move( g_messaging );
            g_roomTopics.clear();
            g_derivedTopics.clear();
            g_explicitTopics.clear();
        }
        if ( lock != nullptr )
        {
            lock->unlock();
        }
        else if ( exitLock.owns_lock() )
        {
            exitLock.unlock();
        }
        if ( sdkBootedHere )
        {
            GeniusSDKShutdown();
        }
        if ( session != nullptr )
        {
            // Destroy the messaging component FIRST (WR-01): its destructor
            // drains + joins the archive worker, and the drain's Puts must
            // land on a STILL-RUNNING store — shutting the session down
            // first makes GcsGlobalDb::Put's m_running gate drop every
            // queued write (local message loss). The receive-intake gate
            // plus the in-flight drain in gcs_shutdown guarantee nothing
            // re-enqueues or calls into Messaging past this point. The
            // entity catalog is destroyed after the drain but still before
            // the session it borrows.
            messaging.reset();
            session->Shutdown();
            entities.reset();
            session.reset();
        }
        if ( lock != nullptr )
        {
            lock->lock();
            g_tearingDown.store( false );
        }
    }

    /**
     * \brief Guarantees a booted GeniusSDK node for the store (D-20 ordering).
     *
     * GcsGlobalDb::Initialize requires GeniusSDKGetNode() to be non-null, and
     * nothing on the Dart side of this four-function ABI boots the SDK — so
     * gcs_init boots it here. A node that already exists (a host harness
     * booted one, e.g. the C++ test fixtures) is reused as-is and its lifetime
     * is never touched by this library. Otherwise the node boots under the
     * db_path's parent directory with the offline-safe dev config. An empty
     * mnemonic (the GcsConfig default) keeps GeniusSDKInit's wallet contract:
     * reuse the wallet persisted under the base path, creating a child wallet
     * when none exists — that child wallet connects to a parent wallet through
     * other mechanisms (Child Wallets, later phase). A non-empty mnemonic
     * boots the provided account's wallet instead. The mnemonic pointer is
     * consumed inside the call (the SDK copies it); nothing is retained.
     *
     * \param[in] config The parsed GcsConfig carrying db_path and mnemonic.
     * \return true when a node is running on return; false when the SDK could
     *         not boot (gcs_init must fail).
     */
    bool EnsureSdkBooted( const gcs::chat::GcsConfig& config )
    {
        if ( GeniusSDKGetNode() != nullptr )
        {
            return true; // externally booted — reuse it and leave its lifetime alone
        }

        const std::filesystem::path basePath = SessionBasePath( config );
        std::error_code createEc;
        std::filesystem::create_directories( basePath, createEc ); // best-effort; the SDK reports real failures

        const std::string basePathString = basePath.string();
        const char* initPath = nullptr;
        if ( config.mnemonic().empty() )
        {
            initPath = GeniusSDKInit( basePathString.c_str(), kDevConfig );
        }
        else
        {
            initPath = GeniusSDKInitWithMnemonic( basePathString.c_str(), kDevConfig, config.mnemonic().c_str() );
        }
        if ( initPath == nullptr )
        {
            spdlog::error( "gcs_ffi: embedded GeniusSDK node failed to boot under '{}'", basePathString );
            return false;
        }
        g_sdkBootedHere = true;
        spdlog::info( "gcs_ffi: booted embedded GeniusSDK node under '{}' (child wallet created when none present)",
                      basePathString );
        // Exit-time pairing for that boot. A macOS app quit never runs the
        // Dart dispose path (NSApplication terminate: goes straight through
        // exit()), so gcs_shutdown is never called and the SDK's static
        // shared_ptr<GeniusNode> destructor destroys a STILL-RUNNING node at
        // exit — ~GeniusNode throws with its worker threads alive and the
        // process dies with SIGABRT (crash report 2026-09-19). Registering
        // here (at gcs_init, later than the dylib-load-time static
        // destructor) means LIFO runs this hook FIRST, while the process
        // world is intact; the static destructor afterwards sees an empty
        // shared_ptr. One registration per process — the hook re-checks the
        // pairing flag when it actually fires.
        if ( !g_exitHookRegistered )
        {
            g_exitHookRegistered = ( std::atexit( [] { TeardownSessionAndNode( nullptr ); } ) == 0 );
            if ( !g_exitHookRegistered )
            {
                spdlog::error( "gcs_ffi: std::atexit registration failed — quitting the host app will crash at exit" );
            }
        }
        return true;
    }

    /**
     * \brief Serializes a GcsEvent envelope to codec-encoded bytes.
     *
     * \param[in] event  The event envelope to serialize.
     * \return The serialized protobuf bytes.
     */
    std::string SerializeGcsEvent( const gcs::chat::GcsEvent& event )
    {
        std::string bytes;
        if ( !event.SerializeToString( &bytes ) )
        {
            // Nearly impossible in proto3, but the failure must not be
            // silent: an empty/partial buffer would reach the Dart port and
            // fail to parse there with no diagnostic (IN-02).
            spdlog::error( "gcs_ffi: GcsEvent SerializeToString failed — posting empty bytes" );
        }
        return bytes;
    }

    /**
     * \brief Pushes a GcsEvent envelope to the registered Dart port.
     *
     * Serializes the event and posts it as a Dart_CObject typed-data (uint8)
     * message through the Dart API_DL indirection (D-26: protobuf bytes, never
     * a raw string). Callers must hold g_mutex when the event is built from
     * guarded state.
     *
     * \param[in] event  The event envelope to push.
     */
    void PostToDart( const gcs::chat::GcsEvent& event )
    {
        const std::string bytes = SerializeGcsEvent( event );
        const int64_t port = g_dartPort.load();
        if ( port == 0 || Dart_PostCObject_DL == nullptr )
        {
            return; // no registered port / API_DL table not initialized (no Dart VM attached)
        }

        // The local bytes buffer outlives the synchronous post — the Dart VM
        // copies typed-data before returning, so no dangling buffer (T-01-05-02).
        Dart_CObject message{};
        message.type = Dart_CObject_kTypedData;
        message.value.as_typed_data.type = Dart_TypedData_kUint8;
        message.value.as_typed_data.length = static_cast<intptr_t>( bytes.size() );
        message.value.as_typed_data.values = reinterpret_cast<const uint8_t*>( bytes.data() );
        // A closed/failing port returns false — safe to ignore (Pitfall 6 / T-01-05-01).
        (void)Dart_PostCObject_DL( port, &message );
    }

    /**
     * \brief Builds a RoomList event from the current joined-topic set.
     *
     * Callers must hold g_mutex (reads g_roomTopics).
     *
     * \return A GcsEvent envelope carrying the RoomList.
     */
    gcs::chat::GcsEvent BuildRoomListEvent()
    {
        gcs::chat::GcsEvent event;
        gcs::chat::RoomList* roomList = event.mutable_room_list();
        for ( const std::string& topic : g_roomTopics )
        {
            roomList->add_room_topic( topic );
        }
        return event;
    }

    /**
     * \brief Builds a SpaceTree event from the entity catalog (D-02).
     *
     * Flat records — Dart builds the tree. Tombstoned entities are already
     * skipped by EntityStore::Spaces()/Rooms(). Callers must hold g_mutex
     * (reads g_entities).
     *
     * \return A GcsEvent envelope carrying the SpaceTree.
     */
    gcs::chat::GcsEvent BuildSpaceTreeEvent()
    {
        gcs::chat::GcsEvent event;
        gcs::chat::SpaceTree* tree = event.mutable_space_tree();
        for ( const gcs::chat::SpaceRecord& space : g_entities->Spaces() )
        {
            *tree->add_space() = space;
        }
        for ( const gcs::chat::RoomRecord& room : g_entities->Rooms() )
        {
            *tree->add_room() = room;
        }
        return event;
    }

    /**
     * \brief Pushes the room's converged history as a MessageHistory batch.
     *
     * Callers must hold g_mutex (the sink serializes under it). This is the
     * sanctioned under-mutex Messaging call (IN-01): QueryHistory never
     * pushes through the sink and its GlobalDB scan is a converged local
     * read, so it cannot participate in the 03-06 ABBA cycle. The returned
     * MessageHistory is already decrypted and role-flipped by
     * Messaging::QueryHistory (D-08), so it is posted verbatim — no FFI-side
     * decrypt or re-mapping. A failed scan is logged only (no error notice).
     *
     * \param[in] roomTopic The room topic to replay.
     */
    void PushMessageHistory( const std::string &roomTopic )
    {
        auto history = g_messaging->QueryHistory( roomTopic );
        if ( !history.has_value() )
        {
            spdlog::error( "gcs_ffi: history scan failed for room '{}'", roomTopic );
            return;
        }
        gcs::chat::GcsEvent event;
        *event.mutable_message_history() = history.value();
        PostToDart( event );
    }

    /**
     * \brief Pushes an ErrorNotice carrying a raw error string (D-29).
     *
     * \param[in] message  The raw error string to push.
     */
    void PostErrorNotice( const std::string& message )
    {
        gcs::chat::GcsEvent event;
        event.mutable_error()->set_message( message );
        PostToDart( event );
    }

    /**
     * \brief Arms the raw GossipSub live subscribe for a room topic (D-03).
     *
     * Callers hold g_mutex via lock, but the subscribe itself runs with the
     * lock RELEASED (CR-01): GcsGlobalDb::Subscribe blocks on the future
     * only the GossipSub strand fulfills, and a strand already delivering
     * into the receive funnel needs g_mutex — holding g_mutex across the
     * .get() is the join-path twin of the 03-06 send-path ABBA deadlock.
     * The session pointer is safe across the unlocked window because the
     * call is counted in g_inFlight and gcs_shutdown drains that count
     * before evicting the globals (WR-02). The callback fires on the
     * GossipPubSub strand thread (not the FFI command thread), so the
     * lambda checks the teardown intake gate atomically and takes g_mutex
     * ONLY to copy the Messaging pointer, calling OnLiveMessage outside the
     * lock (ABBA fix, 03-06; T-03-14).
     *
     * \param[in] lock      The caller's held unique_lock of g_mutex.
     * \param[in] roomTopic The room topic to subscribe to.
     * \return true when the live subscription armed successfully; false after
     *         a failed subscribe, which also pushes an ErrorNotice (WR-03).
     */
    bool SubscribeLive( std::unique_lock<std::mutex> &lock, const std::string &roomTopic )
    {
        gcs::CoreSession* session = g_session.get();
        g_inFlight += 1;
        lock.unlock();
        const bool liveOk = session->Subscribe( roomTopic,
                                                []( const std::string &topic, const std::string &data )
                                                {
                                                    if ( g_receivingDisabled.load() )
                                                    {
                                                        return; // teardown intake gate (CR-02)
                                                    }
                                                    gcs::Messaging* messaging = nullptr;
                                                    {
                                                        std::lock_guard<std::mutex> lock( g_mutex );
                                                        messaging = g_messaging.get();
                                                        if ( messaging != nullptr )
                                                        {
                                                            g_inFlight += 1; // counted until the funnel returns (WR-02)
                                                        }
                                                    }
                                                    if ( messaging == nullptr )
                                                    {
                                                        return;
                                                    }
                                                    messaging->OnLiveMessage( topic, data );
                                                    {
                                                        std::lock_guard<std::mutex> lock( g_mutex );
                                                        g_inFlight -= 1;
                                                    }
                                                    g_idleCond.notify_all();
                                                } )
                                .has_value();
        lock.lock();
        g_inFlight -= 1;
        g_idleCond.notify_all();
        if ( !liveOk )
        {
            spdlog::error( "gcs_ffi: live subscribe failed for room '{}'", roomTopic );
            // WR-03: surface the partial failure on the push port like every
            // other failure on the join path — the room otherwise appears
            // joined while live delivery is dead (CRDT heal only).
            PostErrorNotice( "live subscribe failed for room '" + roomTopic + "'" );
        }
        return liveOk;
    }

    /**
     * \brief Recomputes the derived-join topic set (D-04) and syncs it into the
     *        session registrations and the joined-topic projection.
     *
     * Derived joins are pure local recomputation over the catalog: every room
     * whose parent space has autoJoinRooms. Newly derived topics register
     * listen-first then broadcast (D-07) and enter g_derivedTopics and (once)
     * g_roomTopics so the pushed RoomList reflects them. A topic that left the
     * derived set (autoJoinRooms toggled false) leaves BOTH vectors — the
     * RoomList projection drops it while the underlying pubsub registration
     * stays sticky (GcsGlobalDb has no Remove*Topic; harmless pre-messaging —
     * Pitfall 4) — UNLESS the client also joined it explicitly via
     * join_topic: explicit membership outranks derivation and is never
     * revoked by derived eviction (WR-03). Smoke topics are never in
     * g_derivedTopics and are never touched here. Callers hold g_mutex via
     * lock (mutates g_entities' registrations via g_session, g_derivedTopics,
     * g_roomTopics, g_explicitTopics); the lock is released only around the
     * blocking live subscribes (CR-01 — see SubscribeLive).
     *
     * \param[in] lock The caller's held unique_lock of g_mutex.
     */
    void RefreshDerivedJoins( std::unique_lock<std::mutex> &lock )
    {
        const std::vector<std::string> derived = g_entities->DerivedJoinedTopics();

        // Register newly derived topics (listen first, then broadcast — D-07,
        // the same ordering GcsGlobalDb::Initialize and join_topic use).
        for ( const std::string& topic : derived )
        {
            if ( std::find( g_derivedTopics.begin(), g_derivedTopics.end(), topic )
                 != g_derivedTopics.end() )
            {
                continue;
            }
            if ( !g_session->AddListenTopic( topic ).has_value()
                 || !g_session->AddBroadcastTopic( topic ).has_value() )
            {
                spdlog::error( "gcs_ffi: derived join topic '{}' failed to register — "
                               "not added to the room list",
                               topic );
                continue; // registration failed — keep it out of the projection
            }
            g_derivedTopics.push_back( topic );
            if ( std::find( g_roomTopics.begin(), g_roomTopics.end(), topic )
                 == g_roomTopics.end() )
            {
                g_roomTopics.push_back( topic );
            }
            // D-06: a newly derived (auto-joined) room replays its history and
            // arms the live subscribe so it receives live messages (derived joins).
            PushMessageHistory( topic );
            SubscribeLive( lock, topic );
        }

        // Topics no longer derived leave the projection only (Pitfall 4) —
        // unless the client joined them explicitly (WR-03): an explicit join
        // outranks derivation, so eviction never revokes it from g_roomTopics.
        std::vector<std::string> stillDerived;
        for ( const std::string& topic : g_derivedTopics )
        {
            if ( std::find( derived.begin(), derived.end(), topic ) != derived.end() )
            {
                stillDerived.push_back( topic );
            }
            else if ( std::find( g_explicitTopics.begin(), g_explicitTopics.end(), topic )
                      == g_explicitTopics.end() )
            {
                g_roomTopics.erase( std::remove( g_roomTopics.begin(), g_roomTopics.end(), topic ),
                                    g_roomTopics.end() );
            }
        }
        g_derivedTopics = std::move( stillDerived );
    }

} // namespace

extern "C"
{
    GCS_FFI_API GcsSession* gcs_init( const uint8_t* configBytes, size_t configLength ) GCS_FFI_NOEXCEPT
    {
        // unique_lock (not lock_guard): RefreshDerivedJoins releases the
        // lock around each blocking live subscribe (CR-01 — see
        // SubscribeLive) and needs to re-acquire it afterwards.
        std::unique_lock<std::mutex> lock( g_mutex );

        if ( g_tearingDown.load() )
        {
            return nullptr; // a teardown's unlocked destruction phase is mid-flight (CR-02)
        }

        // One-time Dart API_DL state check (per-process init contract of
        // dart_api_dl.h). DEVIATION from plan text (Rule 1): the plan calls for
        // Dart_InitializeApiDL(nullptr) here, but the vendored SDK source
        // dereferences its argument unconditionally (dart_api_dl.c —
        // `dart_api_data->major`), so a null table pointer would fault. The
        // Dart VM's table pointer (NativeApi.initializeApiDLData) is not carried
        // by the four-function ABI; the Dart side initializes the table by
        // calling this library's exported Dart_InitializeApiDL symbol (verified
        // exported from libgcs_ffi.dylib). Until that runs, Dart_PostCObject_DL
        // stays null and PostToDart is a safe no-op — non-callback FFI calls
        // remain fully usable (the plan's log-and-continue outcome).
        if ( !g_apiDlInitialized.exchange( true ) )
        {
            if ( Dart_PostCObject_DL == nullptr )
            {
                spdlog::error( "gcs_ffi: Dart API_DL table not initialized — pushed GcsEvent "
                               "delivery disabled until Dart calls Dart_InitializeApiDL" );
            }
        }

        if ( g_session )
        {
            return reinterpret_cast<GcsSession*>( g_session.get() ); // idempotent
        }
        if ( configBytes == nullptr || configLength == 0 )
        {
            return nullptr;
        }
        if ( configLength > static_cast<size_t>( std::numeric_limits<int>::max() ) )
        {
            return nullptr; // unvalidated size_t->int narrowing at the ABI boundary (IN-01)
        }

        gcs::chat::GcsConfig config;
        if ( !config.ParseFromArray( configBytes, static_cast<int>( configLength ) ) )
        {
            return nullptr; // untrusted bytes never reach partially-parsed state
        }
        if ( config.codec() != gcs::chat::CODEC_PROTOBUF )
        {
            return nullptr; // D-29: codec bound at creation, immutable for the store's lifetime
        }

        // GCS component logs mirror to a rotating gcs_chat.log beside the
        // store — file logs are the post-hoc diagnostic surface for packaged
        // apps (the node's own sgnslog.log lands under the same base).
        // Configured BEFORE the node boot so boot failures are captured.
        sgns::gcs::SetGcsFileLogBasePath( SessionBasePath( config ) );
        sgns::gcs::ApplyFileSinkToDefaultLogger();

        // D-20 ordering: the store needs a booted GeniusSDK node and nothing
        // on the Dart side of this ABI boots one — boot the embedded node here
        // (reusing an externally booted node when present).
        if ( !EnsureSdkBooted( config ) )
        {
            return nullptr;
        }

        gcs::CoreSession::Config coreConfig{};
        coreConfig.m_dbPath = config.db_path(); // empty allowed — store default

        auto session = std::make_unique<gcs::CoreSession>( std::move( coreConfig ) );
        if ( !session->Initialize().has_value() )
        {
            // IN-02: pair the embedded boot this gcs_init performed — the
            // node (wallet, threads, ports) must not keep running after a
            // failed init leaves no session owning it. Mirrors the smoke-topic
            // failure path below (Shutdown is an idempotent no-op for a
            // session whose Initialize failed); resetting the pairing flag
            // keeps a later successful boot/shutdown cycle balanced.
            session->Shutdown();
            if ( g_sdkBootedHere )
            {
                g_sdkBootedHere = false;
                GeniusSDKShutdown();
            }
            return nullptr;
        }

        // D-26: pre-join the smoke-topic set so the pushed room list is
        // non-empty; only topics that joined both ways are listed (listen
        // first, then broadcast — GcsGlobalDb::Initialize's D-07 ordering).
        bool joinedAnySmokeTopic = false;
        for ( const char* smokeTopic : { kSmokeTopicA, kSmokeTopicB } )
        {
            if ( session->AddListenTopic( smokeTopic ).has_value()
                 && session->AddBroadcastTopic( smokeTopic ).has_value() )
            {
                g_roomTopics.push_back( smokeTopic );
                joinedAnySmokeTopic = true;
            }
            else
            {
                spdlog::error( "gcs_ffi: smoke topic '{}' failed to join during init", smokeTopic );
            }
        }
        // A session that joined NO smoke topic cannot honor the non-empty
        // room-list contract (gcs_core.h) — fail init instead of returning a
        // silently degraded session the caller cannot distinguish from a
        // healthy one once Readiness(ready=true) is pushed.
        if ( !joinedAnySmokeTopic )
        {
            spdlog::error( "gcs_ffi: no smoke topic could be joined — failing gcs_init" );
            session->Shutdown();
            return nullptr;
        }

        g_session = std::move( session );

        // Re-arm receive intake: a prior gcs_shutdown latched the teardown
        // gate closed (CR-02) — the fresh session's receive callbacks funnel.
        g_receivingDisabled.store( false );

        // Phase 3 (D-08): construct the messaging component with the REAL
        // vendored-OpenSSL crypto seam injected and encryption enabled by
        // default for all Phase 3 rooms. The EventSink self-locks g_mutex
        // around PostToDart — the precise ABBA rule (03-06, IN-01) is that
        // no Messaging method that PUSHES via the sink may run under
        // g_mutex (see the kSendText arm and the receive lambdas), so the
        // sink must provide its own serialization. Messaging::QueryHistory
        // via PushMessageHistory is the sanctioned exception: it never
        // invokes the sink and its GlobalDB scan is a converged local read,
        // so it runs under g_mutex on the join paths (a future scan that
        // waits on the CRDT job pipeline would have to move out from under
        // the mutex). The FFI names the adapter functions here but never
        // calls EVP directly — encryption/decryption happen inside
        // Messaging, and the FFI/Dart only push/see plaintext events (D-08).
        {
            gcs::Messaging::CryptoSeam cryptoSeam;
            cryptoSeam.encrypt = &gcs::crypto::EncryptPayload;
            cryptoSeam.decrypt = &gcs::crypto::DecryptPayload;
            cryptoSeam.enabled = true;
            g_messaging = std::make_unique<gcs::Messaging>(
                *g_session, std::string( GeniusSDKGetAddress().address ),
                []( const gcs::chat::GcsEvent &event )
                {
                    std::lock_guard<std::mutex> lock( g_mutex );
                    PostToDart( event );
                },
                cryptoSeam );
        }

        // Bridge CRDT-synced message arrivals into the Messaging receive funnel
        // (D-03 heal path). The callback fires on the io/DagWorker thread, so
        // it checks the teardown intake gate ATOMICALLY (CR-02) and takes
        // g_mutex ONLY to copy the Messaging pointer, calling OnMessageArrived
        // outside the lock — holding g_mutex across a Messaging call can
        // deadlock against a command thread blocked in SendMessage's
        // WaitForJob (ABBA fix, 03-06; T-03-14). Decryption happens inside
        // the Messaging funnel (D-08).
        if ( !g_session->RegisterNewElementCallback(
                   gcs::Messaging::kMessagesKeyCallbackPattern,
                   []( const std::string &key, const std::string &value )
                   {
                       if ( g_receivingDisabled.load() )
                       {
                           return; // teardown intake gate (CR-02)
                       }
                       gcs::Messaging* messaging = nullptr;
                       {
                           std::lock_guard<std::mutex> lock( g_mutex );
                           messaging = g_messaging.get();
                           if ( messaging != nullptr )
                           {
                               g_inFlight += 1; // counted until the funnel returns (WR-02)
                           }
                       }
                       if ( messaging == nullptr )
                       {
                           return;
                       }
                       messaging->OnMessageArrived( key, value );
                       {
                           std::lock_guard<std::mutex> lock( g_mutex );
                           g_inFlight -= 1;
                       }
                       g_idleCond.notify_all();
                   } )
                  .has_value() )
        {
            spdlog::error( "gcs_ffi: message receive-callback registration failed" );
        }

        // Phase 2: replay the persisted entity catalog (D-02). Construct the
        // store ONLY after the g_session move — RefreshDerivedJoins uses the
        // g_session global. A load failure is logged and never fails init:
        // LoadFromStore treats a missing manifest as an empty catalog, so an
        // error here still leaves a usable session with an empty catalog. No
        // pushes in this block — the Dart port is not yet registered, so posts
        // would be silent no-ops (Pitfall 9: SpaceTree pushes live in
        // gcs_subscribe and after mutations).
        g_entities = std::make_unique<gcs::EntityStore>( *g_session );
        if ( g_entities->LoadFromStore().has_error() )
        {
            spdlog::error( "gcs_ffi: entity catalog load failed — continuing with an empty catalog" );
        }
        RefreshDerivedJoins( lock );

        return reinterpret_cast<GcsSession*>( g_session.get() );
    }

    GCS_FFI_API int gcs_publish( GcsSession* session, const char* topic,
                                 const uint8_t* payloadBytes, size_t payloadLength ) GCS_FFI_NOEXCEPT
    {
        // unique_lock (not lock_guard): the kSendText arm releases g_mutex
        // across Messaging::SendMessage — its topics-aware archive Put blocks
        // in WaitForJob, and the job pipeline fires the CRDT heal callback on
        // the DagWorker thread, whose lambda takes g_mutex. Holding g_mutex
        // across the blocking Put is an ABBA deadlock (03-06 multinode test).
        std::unique_lock<std::mutex> lock( g_mutex );

        if ( g_session == nullptr || reinterpret_cast<gcs::CoreSession*>( session ) != g_session.get()
             || topic == nullptr || payloadBytes == nullptr || payloadLength == 0 )
        {
            // payloadLength zero is invalid per the gcs_publish doc contract —
            // reject it as an argument error instead of letting it parse as an
            // empty proto and surface later as PAYLOAD_NOT_SET (IN-03).
            return GCS_ERROR_INVALID_ARGUMENT;
        }
        if ( payloadLength > static_cast<size_t>( std::numeric_limits<int>::max() ) )
        {
            return GCS_ERROR_INVALID_ARGUMENT; // unvalidated size_t->int narrowing at the ABI boundary (IN-01)
        }
        if ( !g_session->IsRunning() )
        {
            return GCS_ERROR_NOT_RUNNING;
        }
        if ( std::strcmp( topic, kCommandTopic ) != 0 )
        {
            return GCS_ERROR_INVALID_ARGUMENT; // Phase 1: the store's namespace grows in later phases
        }

        // Copy the caller-owned buffer inside the lock — the caller owns it only for
        // the call duration and no pointer to it is retained after return.
        const std::string payload( reinterpret_cast<const char*>( payloadBytes ), payloadLength );

        gcs::chat::GcsCommand command;
        if ( !command.ParseFromArray( payload.data(), static_cast<int>( payload.size() ) ) )
        {
            PostErrorNotice( "GcsCommand parse failed" ); // D-29: raw error string on the push port
            return GCS_ERROR_INVALID_ARGUMENT;
        }

        switch ( command.payload_case() )
        {
        case gcs::chat::GcsCommand::kJoinTopic:
        {
            const std::string roomTopic = command.join_topic().room_topic();
            if ( roomTopic.empty() )
            {
                PostErrorNotice( "join_topic rejected: room_topic is empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( roomTopic.size() > kMaxTopicLength )
            {
                PostErrorNotice( "join_topic rejected: room_topic exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( !IsValidRoomTopicShape( roomTopic ) )
            {
                // WR-03: a topic with an extra '/' (or outside the gcs/chat/
                // namespace) makes the archive key grammar ambiguous — a
                // prefix scan for one room would return another room's records.
                PostErrorNotice( "join_topic rejected: room_topic must be 'gcs/chat/<id>' "
                                 "with no additional '/'" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // Listen first, then broadcast — the same ordering
            // GcsGlobalDb::Initialize uses (D-07). A listen failure leaves
            // nothing registered; a broadcast failure leaves only the listen
            // registration (CoreSession has no Remove*Topic pass-through to
            // roll it back in Phase 1), but g_roomTopics is NOT mutated and
            // the failure is surfaced, so the pushed RoomList keeps matching
            // every topic that joined BOTH ways.
            if ( !g_session->AddListenTopic( roomTopic ).has_value()
                 || !g_session->AddBroadcastTopic( roomTopic ).has_value() )
            {
                spdlog::error( "gcs_ffi: join_topic('{}') failed — topic not "
                               "added to the room list",
                               roomTopic );
                PostErrorNotice( "join_topic failed for room '" + roomTopic + "'" ); // D-29: raw error string on the push port
                return GCS_ERROR_GENERIC;
            }
            // Idempotent room list: a repeated join of an already-joined topic
            // (Dart-side retry, double-tap) must not append a duplicate room.
            if ( std::find( g_roomTopics.begin(), g_roomTopics.end(), roomTopic ) == g_roomTopics.end() )
            {
                g_roomTopics.push_back( roomTopic );
            }
            // Track explicit provenance (WR-03): RefreshDerivedJoins must
            // never evict an explicitly-joined topic from g_roomTopics when
            // its derivation ends.
            if ( std::find( g_explicitTopics.begin(), g_explicitTopics.end(), roomTopic )
                 == g_explicitTopics.end() )
            {
                g_explicitTopics.push_back( roomTopic );
            }
            PostToDart( BuildRoomListEvent() );
            // D-06: replay the room's history as a pushed MessageHistory batch,
            // THEN arm the raw live subscribe so any live message lands after
            // the batch and is absorbed by the id-keyed dedupe (Pitfall 3).
            PushMessageHistory( roomTopic );
            if ( !SubscribeLive( lock, roomTopic ) )
            {
                // WR-03: the join itself registered (the RoomList with the
                // topic was already pushed), but live delivery is dead —
                // mirror the arm's other partial failures: the ErrorNotice is
                // pushed inside SubscribeLive, the error code returned here.
                return GCS_ERROR_GENERIC;
            }
            return GCS_OK;
        }
        case gcs::chat::GcsCommand::kSendText:
        {
            const gcs::chat::SendTextCommand& sendText = command.send_text();
            if ( sendText.room_topic().empty() )
            {
                PostErrorNotice( "send_text rejected: room_topic is empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( sendText.room_topic().size() > kMaxTopicLength )
            {
                PostErrorNotice( "send_text rejected: room_topic exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( !IsValidRoomTopicShape( sendText.room_topic() ) )
            {
                // WR-03: same archive-key grammar guard as join_topic — an
                // ambiguous topic shape never reaches the key space.
                PostErrorNotice( "send_text rejected: room_topic must be 'gcs/chat/<id>' "
                                 "with no additional '/'" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( std::find( g_roomTopics.begin(), g_roomTopics.end(), sendText.room_topic() )
                 == g_roomTopics.end() )
            {
                PostErrorNotice( "send_text rejected: room '" + sendText.room_topic() + "' is not joined" );
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( sendText.text().size() > kMaxMessageTextLength )
            {
                PostErrorNotice( "send_text rejected: text exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // D-03/D-04/D-07/D-08: delegate to the Messaging component, which
            // mints the id, stamps the sender, pushes pending -> live publish +
            // archive -> complete, and encrypts the envelope (production seam).
            // Called WITHOUT g_mutex: the archive Put blocks in WaitForJob and
            // the DagWorker heal lambda takes g_mutex (ABBA fix, 03-06). The
            // raw Messaging pointer is safe across the unlocked window because
            // the call is counted in g_inFlight and gcs_shutdown drains that
            // count to zero before tearing the globals down (WR-02); Messaging
            // is internally synchronized (m_seenMutex/m_archiveMutex).
            gcs::Messaging* messaging = g_messaging.get();
            g_inFlight += 1;
            lock.unlock();
            const bool sendOk = messaging->SendMessage( sendText.room_topic(), sendText.text() ).has_value();
            lock.lock();
            g_inFlight -= 1;
            g_idleCond.notify_all();
            if ( !sendOk )
            {
                PostErrorNotice( "send_text failed for room '" + sendText.room_topic() + "'" );
                return GCS_ERROR_GENERIC;
            }
            return GCS_OK;
        }
        case gcs::chat::GcsCommand::kCreateSpace:
        {
            const gcs::chat::CreateSpaceCommand& createSpace = command.create_space();
            if ( createSpace.name().empty() )
            {
                PostErrorNotice( "create_space rejected: name is empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( Utf8CodePointCount( createSpace.name() ) > kMaxEntityNameLength )
            {
                PostErrorNotice( "create_space rejected: name exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // C++ mints the id and stamps every authority field (D-01/D-27);
            // Dart's command is data-only. A new space has no rooms yet, so
            // the derived-join set — and the RoomList — cannot change here.
            if ( !g_entities->CreateSpace( createSpace.name(), createSpace.is_public(),
                                           createSpace.auto_join_rooms() )
                      .has_value() )
            {
                spdlog::error( "gcs_ffi: create_space store write failed for '{}'",
                               createSpace.name() );
                PostErrorNotice( "create_space store write failed for '" + createSpace.name()
                                 + "'" ); // D-29: raw error string on the push port
                return GCS_ERROR_GENERIC;
            }
            PostToDart( BuildSpaceTreeEvent() );
            return GCS_OK;
        }
        case gcs::chat::GcsCommand::kCreateRoom:
        {
            const gcs::chat::CreateRoomCommand& createRoom = command.create_room();
            if ( createRoom.name().empty() )
            {
                PostErrorNotice( "create_room rejected: name is empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( Utf8CodePointCount( createRoom.name() ) > kMaxEntityNameLength )
            {
                PostErrorNotice( "create_room rejected: name exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // Empty parent = standalone room (D-01 unified room model); a
            // non-empty parent must reference a known, non-tombstoned space
            // (T-02-05) — rejected before any store write.
            if ( !createRoom.parent_space_id().empty()
                 && !g_entities->IsValidParentSpace( createRoom.parent_space_id() ) )
            {
                PostErrorNotice( "create_room rejected: parent space '" + createRoom.parent_space_id()
                                 + "' not found" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( !g_entities->CreateRoom( createRoom.name(), createRoom.parent_space_id() ).has_value() )
            {
                spdlog::error( "gcs_ffi: create_room store write failed for '{}'", createRoom.name() );
                PostErrorNotice( "create_room store write failed for '" + createRoom.name()
                                 + "'" ); // D-29: raw error string on the push port
                return GCS_ERROR_GENERIC;
            }
            // The new room may be auto-joined (parent autoJoinRooms) — recompute
            // the derived set so RoomList reflects it (D-04, retroactive by
            // construction).
            RefreshDerivedJoins( lock );
            PostToDart( BuildSpaceTreeEvent() );
            PostToDart( BuildRoomListEvent() );
            return GCS_OK;
        }
        case gcs::chat::GcsCommand::kUpdateSpace:
        {
            const gcs::chat::UpdateSpaceCommand& updateSpace = command.update_space();
            if ( updateSpace.space_id().empty() || updateSpace.name().empty() )
            {
                PostErrorNotice( "update_space rejected: space_id and name must be non-empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            if ( Utf8CodePointCount( updateSpace.name() ) > kMaxEntityNameLength )
            {
                PostErrorNotice( "update_space rejected: name exceeds maximum length" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // Full desired state, never a patch (per-key LWW replaces the whole
            // value); EntityStore preserves the immutable fields from the stored
            // record and re-stamps updated_at_ms.
            gcs::chat::SpaceRecord desired;
            desired.set_id( updateSpace.space_id() );
            desired.set_name( updateSpace.name() );
            desired.set_is_public( updateSpace.is_public() );
            desired.set_auto_join_rooms( updateSpace.auto_join_rooms() );
            if ( !g_entities->UpdateSpace( desired ).has_value() )
            {
                spdlog::error( "gcs_ffi: update_space failed for '{}'", updateSpace.space_id() );
                PostErrorNotice( "update_space failed for '" + updateSpace.space_id()
                                 + "'" ); // D-29: raw error string on the push port
                return GCS_ERROR_GENERIC;
            }
            // An autoJoinRooms toggle changes the derived set (D-04) — recompute
            // so RoomList gains/loses the space's rooms.
            RefreshDerivedJoins( lock );
            PostToDart( BuildSpaceTreeEvent() );
            PostToDart( BuildRoomListEvent() );
            return GCS_OK;
        }
        case gcs::chat::GcsCommand::PAYLOAD_NOT_SET:
        default:
        {
            PostErrorNotice( "GcsCommand payload not set" );
            return GCS_ERROR_INVALID_ARGUMENT;
        }
        }
    }

    GCS_FFI_API int gcs_subscribe( GcsSession* session, const char* topic, int64_t dartPort ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( g_session == nullptr || reinterpret_cast<gcs::CoreSession*>( session ) != g_session.get()
             || topic == nullptr || dartPort == 0 )
        {
            return GCS_ERROR_INVALID_ARGUMENT;
        }
        if ( !g_session->IsRunning() )
        {
            return GCS_ERROR_NOT_RUNNING;
        }

        g_dartPort = dartPort;

        // D-05/D-26 push-not-pull: SpaceTree (catalog) first, then RoomList
        // (joined membership), then Readiness(ready=true) — tree before
        // membership before readiness (D-02), so Dart can render the catalog
        // while not ready. Phase 1 delivers the single event stream to the
        // port regardless of the topic argument; topic-graded delivery is a
        // later-phase refinement.
        PostToDart( BuildSpaceTreeEvent() );
        PostToDart( BuildRoomListEvent() );

        gcs::chat::GcsEvent readyEvent;
        readyEvent.mutable_readiness()->set_ready( true );
        PostToDart( readyEvent );
        return GCS_OK;
    }

    GCS_FFI_API void gcs_shutdown( GcsSession* session ) GCS_FFI_NOEXCEPT
    {
        // unique_lock (not lock_guard): the in-flight drain below waits on
        // g_idleCond, which releases g_mutex while waiting so the counted
        // operations can come back under the mutex and finish (WR-02).
        std::unique_lock<std::mutex> lock( g_mutex );

        if ( g_session != nullptr && reinterpret_cast<gcs::CoreSession*>( session ) == g_session.get() )
        {
            // CR-02: stop receive intake BEFORE draining and tearing down —
            // the gate is atomic so the pubsub and DagWorker bridge callbacks
            // observe it without ever parking on g_mutex, and nothing
            // re-funnels (or re-enqueues archive work) mid-teardown.
            g_receivingDisabled.store( true );
            // WR-02: wait until every operation running unlocked against the
            // session/messaging globals (send_text, live subscribes, receive
            // funnels) is back under the mutex BEFORE teardown destroys those
            // objects — this makes the unlocked raw-pointer windows safe
            // against a concurrent gcs_shutdown instead of assuming the Dart
            // isolate serializes commands against shutdown.
            g_idleCond.wait( lock, [] { return g_inFlight == 0; } );
            // Shared teardown body — also pairs the gcs_init boot: the
            // embedded node goes down only when this library booted it. The
            // lock is passed so the blocking teardown runs OFF g_mutex.
            TeardownSessionAndNode( &lock );
        }
    }
} // extern "C"
