/**
 * @file       gcs_core.h
 * @brief      Minimal topic pub/sub C ABI for the GCS chat core (D-01, D-02, D-27, D-29).
 * @details    Four functions: init / shutdown / publish / subscribe. Data crosses
 *             this boundary as codec-tagged bytes (const uint8_t* + length, never
 *             char*); the codec is bound per store at creation via the serialized
 *             gcs.chat.GcsConfig carried by the init bytes and is immutable for
 *             the store's lifetime (Phase 1: CODEC_PROTOBUF only). Topic-name
 *             parameters are the sole char* exception — they are addressing
 *             metadata, not payloads. There is no pull-style query function and
 *             no per-feature call function: all state and all data (messages,
 *             room-list, readiness, and raw error strings) arrive PUSHED on the
 *             subscribed Dart NativePort as serialized gcs.chat.GcsEvent bytes.
 * @date       2026-08-26
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_CORE_FFI_H
#define GCS_CORE_FFI_H

#include <stddef.h>
#include <stdint.h>

#if defined( _WIN32 )
#if defined( GCS_FFI_EXPORTS )
#define GCS_FFI_API __declspec( dllexport )
#else
#define GCS_FFI_API __declspec( dllimport )
#endif
#else
#define GCS_FFI_API
#endif

#if defined( __cplusplus )
#define GCS_FFI_NOEXCEPT noexcept
extern "C"
{
#else
#define GCS_FFI_NOEXCEPT
#endif

    /** Opaque session handle — declared only, defined in gcs_core_ffi.cpp. */
    typedef struct GcsSession GcsSession;

    /**
     * C status codes returned by the int-valued ABI functions.
     */
    typedef enum GcsStatus
    {
        GCS_OK = 0,
        GCS_ERROR_GENERIC,
        GCS_ERROR_INVALID_ARGUMENT,
        GCS_ERROR_NOT_RUNNING,
        GCS_ERROR_UNSUPPORTED_CODEC
    } GcsStatus;

    /**
     * \brief Creates the session/store from a serialized gcs.chat.GcsConfig
     * (codec-tagged bytes — D-29).
     *
     * The codec is bound to the store here and is immutable for the store's
     * lifetime; Phase 1 accepts CODEC_PROTOBUF only (else the call fails as an
     * unsupported codec). On success the session pre-joins the smoke-topic set
     * so the pushed room list is non-empty.
     *
     * Thread-safe via global mutex; idempotent — when a session already exists
     * its handle is returned again. Returns NULL on failure (invalid arguments,
     * config parse failure, unsupported codec, or initialization failure). Null
     * configBytes is invalid.
     *
     * \param[in] configBytes   Serialized gcs.chat.GcsConfig bytes (caller owns
     *                          the buffer for the call duration only).
     * \param[in] configLength  Length of configBytes in bytes; zero is invalid.
     * \return Opaque session handle on success, NULL on failure.
     */
    GCS_FFI_API GcsSession* gcs_init( const uint8_t* configBytes, size_t configLength ) GCS_FFI_NOEXCEPT;

    /**
     * \brief Publishes a codec-encoded payload to a topic (D-27: commands are
     * topic publishes).
     *
     * Payloads are bytes + length, never char*. On the command topic the payload
     * must parse as gcs.chat.GcsCommand; C++ validates, stamps authority fields,
     * and pushes resulting GcsEvent bytes to the subscribed port. Parse failure
     * returns GCS_ERROR_INVALID_ARGUMENT and pushes an ErrorNotice carrying the
     * raw error string. Phase 1 accepts the command topic only.
     *
     * The caller-owned payload buffer is copied inside the call; no pointer to
     * caller memory is retained after return.
     *
     * \param[in] session         Handle returned by a successful init.
     * \param[in] topic           NUL-terminated topic name to publish to.
     * \param[in] payloadBytes    Codec-encoded payload bytes (caller owns the
     *                            buffer for the call duration only).
     * \param[in] payloadLength   Length of payloadBytes in bytes; zero is invalid.
     * \return GCS_OK on success; GCS_ERROR_INVALID_ARGUMENT on null/mismatched
     *         arguments or a parse failure; GCS_ERROR_NOT_RUNNING when the
     *         session is not running; GCS_ERROR_GENERIC otherwise.
     */
    GCS_FFI_API int gcs_publish( GcsSession* session, const char* topic,
                                 const uint8_t* payloadBytes, size_t payloadLength ) GCS_FFI_NOEXCEPT;

    /**
     * \brief Registers the Dart NativePort (int64_t send port) that receives
     * pushed GcsEvent bytes for the topic (D-05/D-26 push-not-pull — messages,
     * room-list, readiness, and raw error strings all arrive here).
     *
     * On registration, immediately pushes the current RoomList and a
     * Readiness(ready=true) event. Phase 1 delivers all events to the port as a
     * single event stream regardless of the topic argument; topic-graded
     * delivery is a later-phase refinement. No pull-style query function exists.
     *
     * \param[in] session    Handle returned by a successful init.
     * \param[in] topic      NUL-terminated topic name the port subscribes to.
     * \param[in] dartPort   Dart NativePort (send port) id; zero is invalid.
     * \return GCS_OK on success; GCS_ERROR_INVALID_ARGUMENT on null/mismatched
     *         arguments or a zero port; GCS_ERROR_NOT_RUNNING when the session
     *         is not running.
     */
    GCS_FFI_API int gcs_subscribe( GcsSession* session, const char* topic,
                                   int64_t dartPort ) GCS_FFI_NOEXCEPT;

    /**
     * \brief Tears down the session (unregisters the port before freeing).
     *
     * Clears the registered Dart port first, then shuts down and frees the
     * session. Idempotent and null-tolerant: null or mismatched handles are a
     * no-op (an opaque-handle equality check prevents double-free and
     * use-after-free).
     *
     * \param[in] session  Handle returned by a successful init; may be NULL.
     */
    GCS_FFI_API void gcs_shutdown( GcsSession* session ) GCS_FFI_NOEXCEPT;

#if defined( __cplusplus )
}
#endif

#endif // GCS_CORE_FFI_H
