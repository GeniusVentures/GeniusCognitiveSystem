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

#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace
{
    // Dart -> C++ ingress topic (D-27: commands are topic publishes to one topic).
    constexpr const char* kCommandTopic = "gcs/command";
    // Pre-joined smoke topics (D-26 requires >= 2) so the pushed RoomList is non-empty.
    constexpr const char* kSmokeTopicA = "gcs/chat/smoke-test";
    constexpr const char* kSmokeTopicB = "gcs/chat/smoke-test-2";
    // Prefix for C++-stamped message ids (D-04: authority fields never come from Dart).
    constexpr const char* kMessageIdPrefix = "msg-";

    std::mutex g_mutex;                            // guards g_session + g_roomTopics
    std::unique_ptr<gcs::CoreSession> g_session;   // Phase 1: single global session
    std::vector<std::string> g_roomTopics;         // joined topic set (guarded by g_mutex)
    std::atomic<int64_t> g_dartPort{ 0 };          // registered Dart port (0 = unregistered)
    std::atomic<uint64_t> g_messageSeq{ 0 };       // monotonic message id salt
    // One-shot guard for the per-process Dart API_DL table state check in gcs_init
    // (the table itself is initialized via the exported Dart_InitializeApiDL).
    std::atomic<bool> g_apiDlInitialized{ false };

    /**
     * \brief Serializes a GcsEvent envelope to codec-encoded bytes.
     *
     * \param[in] event  The event envelope to serialize.
     * \return The serialized protobuf bytes.
     */
    std::string SerializeGcsEvent( const gcs::chat::GcsEvent& event )
    {
        std::string bytes;
        event.SerializeToString( &bytes );
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
} // namespace

extern "C"
{
    GCS_FFI_API GcsSession* gcs_init( const uint8_t* configBytes, size_t configLength ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

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

        gcs::chat::GcsConfig config;
        if ( !config.ParseFromArray( configBytes, static_cast<int>( configLength ) ) )
        {
            return nullptr; // untrusted bytes never reach partially-parsed state
        }
        if ( config.codec() != gcs::chat::CODEC_PROTOBUF )
        {
            return nullptr; // D-29: codec bound at creation, immutable for the store's lifetime
        }

        gcs::CoreSession::Config coreConfig{};
        coreConfig.m_dbPath = config.db_path(); // empty allowed — store default

        auto session = std::make_unique<gcs::CoreSession>( std::move( coreConfig ) );
        if ( !session->Initialize().has_value() )
        {
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
        return reinterpret_cast<GcsSession*>( g_session.get() );
    }

    GCS_FFI_API int gcs_publish( GcsSession* session, const char* topic,
                                 const uint8_t* payloadBytes, size_t payloadLength ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( g_session == nullptr || reinterpret_cast<gcs::CoreSession*>( session ) != g_session.get()
             || topic == nullptr || payloadBytes == nullptr )
        {
            return GCS_ERROR_INVALID_ARGUMENT;
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
            PostToDart( BuildRoomListEvent() );
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
            if ( std::find( g_roomTopics.begin(), g_roomTopics.end(), sendText.room_topic() )
                 == g_roomTopics.end() )
            {
                PostErrorNotice( "send_text rejected: room '" + sendText.room_topic() + "' is not joined" );
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            gcs::chat::GcsEvent event;
            gcs::chat::ChatMessageState* message = event.mutable_message();
            // D-04: C++ stamps every authority field; Dart's SendTextCommand is data-only.
            message->set_id( kMessageIdPrefix + std::to_string( g_messageSeq.fetch_add( 1 ) ) );
            message->set_room_topic( sendText.room_topic() );
            message->set_role( gcs::chat::MESSAGE_ROLE_USER_SELF );
            message->set_state( gcs::chat::MESSAGE_STATE_COMPLETE );
            message->set_text( sendText.text() );
            message->set_timestamp( std::chrono::duration_cast<std::chrono::milliseconds>(
                                        std::chrono::system_clock::now().time_since_epoch() )
                                        .count() );

            // Phase 1 echo: store the authoritative record, then push it to the port
            // (the real pub/sub flow lands in Phase 3). Key by the C++-stamped
            // authority id (D-04) so per-room history survives — a room-topic-only
            // key made each new message overwrite the previous one; store the
            // payload message, not the push envelope.
            if ( !g_session->Put( sendText.room_topic() + "/" + message->id(),
                                  message->SerializeAsString() ).has_value() )
            {
                return GCS_ERROR_GENERIC;
            }
            PostToDart( event );
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

        // D-05/D-26 push-not-pull: RoomList first, then Readiness(ready=true). Phase 1
        // delivers the single event stream to the port regardless of the topic argument;
        // topic-graded delivery is a later-phase refinement.
        PostToDart( BuildRoomListEvent() );

        gcs::chat::GcsEvent readyEvent;
        readyEvent.mutable_readiness()->set_ready( true );
        PostToDart( readyEvent );
        return GCS_OK;
    }

    GCS_FFI_API void gcs_shutdown( GcsSession* session ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( g_session != nullptr && reinterpret_cast<gcs::CoreSession*>( session ) == g_session.get() )
        {
            g_dartPort = 0; // unregister the port BEFORE teardown
            g_session->Shutdown();
            g_session.reset();
            g_roomTopics.clear();
        }
    }
} // extern "C"
