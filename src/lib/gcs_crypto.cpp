/**
 * @file       gcs_crypto.cpp
 * @brief      Implementation of the stateless gcs::crypto OpenSSL adapter (D-08).
 * @details    The ONLY file in src/ that includes OpenSSL headers (the
 *             `openssl/...` family). Every call
 *             derives a fresh room key (HKDF-SHA256) and constructs a fresh
 *             EVP_CIPHER_CTX / EVP_PKEY_CTX, freeing it on every path (success
 *             or early failure) — callers fire this adapter from the GossipSub
 *             strand, CRDT DagWorker, and FFI command threads concurrently, and
 *             OpenSSL context objects are mutable state machines that must
 *             never be shared. Every EVP/RAND return value is checked; failures
 *             surface as outcome::failure (reusing the GCS GcsDbError domain —
 *             no crypto-specific error code is added this phase).
 * @copyright  (c) 2026 GNUS.AI
 */

#include "gcs_crypto.hpp"

#include <mutex>
#include <optional>
#include <unordered_map>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>

#include "gcs_storage/common/error.hpp" // sgns::gcs::Error (outcome failure domain)

namespace gcs::crypto
{
static_assert( kInviteAuthMarkerLengthBytes == kRoomKeyLengthBytes,
               "invite-auth marker must seal with the 32-byte AES-GCM wrap" );

namespace
{
    /**
     * @brief Derive a key with HKDF-SHA256 using an explicit salt and info.
     *
     * The one HKDF implementation shared by DeriveRoomKey (message keys) and
     * MintInviteAuthenticator (invite-auth keys); a fresh EVP_PKEY_CTX is
     * created and freed on every call.
     *
     * @param[in] ikm         Input keying material bytes.
     * @param[in] ikmSize     Input keying material length.
     * @param[in] salt        HKDF salt bytes.
     * @param[in] saltLength  HKDF salt length.
     * @param[in] info        HKDF info bytes.
     * @param[in] infoLength  HKDF info length.
     * @param[in] length      Desired output key length in bytes.
     * @return outcome::success with the derived key; outcome::failure on any
     *         EVP/HKDF failure or a short derivation.
     */
    outcome::result<std::vector<unsigned char>> DeriveHkdfKey(
        const unsigned char *ikm, int ikmSize, const char *salt,
        std::size_t saltLength, const char *info, std::size_t infoLength,
        std::size_t length )
    {
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id( EVP_PKEY_HKDF, nullptr );
        if ( pctx == nullptr )
        {
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        }

        std::vector<unsigned char> key( length, 0 );
        size_t                     outLen = key.size();
        const int                  ok =
            ( EVP_PKEY_derive_init( pctx ) == 1 )
            && ( EVP_PKEY_CTX_set_hkdf_md( pctx, EVP_sha256() ) == 1 )
            && ( EVP_PKEY_CTX_set1_hkdf_salt(
                     pctx, reinterpret_cast<const unsigned char *>( salt ),
                     static_cast<int>( saltLength ) )
                 == 1 )
            && ( EVP_PKEY_CTX_set1_hkdf_key( pctx, ikm, ikmSize ) == 1 )
            && ( EVP_PKEY_CTX_add1_hkdf_info(
                     pctx, reinterpret_cast<const unsigned char *>( info ),
                     static_cast<int>( infoLength ) )
                 == 1 )
            && ( EVP_PKEY_derive( pctx, key.data(), &outLen ) == 1 )
            && ( outLen == length );

        EVP_PKEY_CTX_free( pctx ); // always

        if ( ok != 1 )
        {
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        }
        return key;
    }

    /**
     * @brief Returns the per-room key store's guarding mutex (D-06).
     *
     * Function-local static so construction is thread-safe and order-independent;
     * guards EVERY read and write of the key map.
     *
     * @return The mutex guarding the per-room key store.
     */
    std::mutex &RoomKeyStoreMutex()
    {
        static std::mutex mutex;
        return mutex;
    }

    /**
     * @brief Returns the process-wide per-room key store (D-06).
     *
     * Maps a room topic to its 32-byte room-key BYTES. Only bytes are shared —
     * never an EVP context (P7) — and every access goes through
     * RoomKeyStoreMutex().
     *
     * @return The per-room key store.
     */
    std::unordered_map<std::string, std::vector<unsigned char>> &RoomKeyStore()
    {
        static std::unordered_map<std::string, std::vector<unsigned char>> store;
        return store;
    }
} // namespace

std::vector<unsigned char> MintRoomKey()
{
    std::vector<unsigned char> key( kRoomKeyLengthBytes, 0 );
    if ( RAND_bytes( key.data(), static_cast<int>( kRoomKeyLengthBytes ) ) != 1 )
    {
        return {}; // documented empty-on-failure contract (header)
    }
    return key;
}

void StoreRoomKey( const std::string &roomTopic, const std::vector<unsigned char> &roomKey )
{
    std::lock_guard<std::mutex> lock( RoomKeyStoreMutex() );
    RoomKeyStore()[ roomTopic ] = roomKey;
}

std::optional<std::vector<unsigned char>> RoomKeyFor( const std::string &roomTopic )
{
    std::lock_guard<std::mutex> lock( RoomKeyStoreMutex() );
    const auto                  it = RoomKeyStore().find( roomTopic );
    if ( it == RoomKeyStore().end() )
    {
        return std::nullopt;
    }
    return it->second;
}

void ClearRoomKey( const std::string &roomTopic )
{
    std::lock_guard<std::mutex> lock( RoomKeyStoreMutex() );
    RoomKeyStore().erase( roomTopic );
}

outcome::result<std::vector<unsigned char>> DeriveRoomKey( const std::string &roomTopic )
{
    // WR-02/D-06: ikm is the member-distributed 32-byte room key when stored,
    // else the room topic string (pre-membership fallback).
    const std::optional<std::vector<unsigned char>> storedRoomKey = RoomKeyFor( roomTopic );
    const unsigned char *ikm     = reinterpret_cast<const unsigned char *>( roomTopic.data() );
    int                  ikmSize = static_cast<int>( roomTopic.size() );
    if ( storedRoomKey.has_value() )
    {
        ikm     = storedRoomKey->data();
        ikmSize = static_cast<int>( storedRoomKey->size() );
    }

    return DeriveHkdfKey( ikm, ikmSize, kMessagesHkdfSalt,
                          sizeof( kMessagesHkdfSalt ) - 1, kMessagesHkdfInfo,
                          sizeof( kMessagesHkdfInfo ) - 1, kRoomKeyLengthBytes );
}

outcome::result<std::string> EncryptPayload( const std::string &roomTopic,
                                             const std::string &plaintext )
{
    auto keyResult = DeriveRoomKey( roomTopic );
    if ( !keyResult.has_value() )
    {
        return keyResult.error();
    }
    const std::vector<unsigned char> &key = keyResult.value();

    unsigned char nonce[ kGcmNonceLength ] = { 0 };
    if ( RAND_bytes( nonce, static_cast<int>( kGcmNonceLength ) ) != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    std::string envelope;
    envelope.reserve( kGcmNonceLength + plaintext.size() + kGcmTagLength );
    envelope.append( reinterpret_cast<const char *>( nonce ), kGcmNonceLength );

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if ( ctx == nullptr )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    int          outLen = 0;
    int          ok     = 0;
    std::string  ciphertext( plaintext.size(), '\0' );
    unsigned char tag[ kGcmTagLength ] = { 0 };

    if ( EVP_EncryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, key.data(), nonce ) == 1
         && EVP_EncryptUpdate( ctx,
                               reinterpret_cast<unsigned char *>( ciphertext.data() ),
                               &outLen,
                               reinterpret_cast<const unsigned char *>( plaintext.data() ),
                               static_cast<int>( plaintext.size() ) )
                == 1
         && EVP_EncryptFinal_ex( ctx, nullptr, &outLen ) == 1 )
    {
        ok = EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_GET_TAG,
                                  static_cast<int>( kGcmTagLength ), tag );
    }

    EVP_CIPHER_CTX_free( ctx ); // always

    if ( ok != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    envelope.append( ciphertext );
    envelope.append( reinterpret_cast<const char *>( tag ), kGcmTagLength );
    return envelope;
}

outcome::result<std::string> DecryptPayload( const std::string &roomTopic,
                                             const std::string &envelope )
{
    if ( envelope.size() < kGcmNonceLength + kGcmTagLength )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    auto keyResult = DeriveRoomKey( roomTopic );
    if ( !keyResult.has_value() )
    {
        return keyResult.error();
    }
    const std::vector<unsigned char> &key = keyResult.value();

    const unsigned char *nonce = reinterpret_cast<const unsigned char *>( envelope.data() );
    const size_t         cipherLen = envelope.size() - kGcmNonceLength - kGcmTagLength;
    const unsigned char *ciphertext = nonce + kGcmNonceLength;
    const unsigned char *tag = ciphertext + cipherLen;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if ( ctx == nullptr )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    int         outLen = 0;
    int         ok     = 0;
    std::string plaintext( cipherLen, '\0' );

    if ( EVP_DecryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, key.data(), nonce ) == 1
         && EVP_DecryptUpdate( ctx,
                               reinterpret_cast<unsigned char *>( plaintext.data() ),
                               &outLen,
                               ciphertext,
                               static_cast<int>( cipherLen ) )
                == 1
         && EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_SET_TAG,
                                 static_cast<int>( kGcmTagLength ),
                                 const_cast<unsigned char *>( tag ) )
                == 1 )
    {
        int finalLen = 0;
        ok = EVP_DecryptFinal_ex( ctx, nullptr, &finalLen );
    }

    EVP_CIPHER_CTX_free( ctx ); // always

    if ( ok != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    return plaintext;
}

outcome::result<std::string> WrapRoomKey( const std::vector<unsigned char> &memberKey,
                                          const std::vector<unsigned char> &roomKey )
{
    if ( memberKey.size() != kRoomKeyLengthBytes || roomKey.size() != kRoomKeyLengthBytes )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    unsigned char nonce[ kGcmNonceLength ] = { 0 };
    if ( RAND_bytes( nonce, static_cast<int>( kGcmNonceLength ) ) != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    std::string envelope;
    envelope.reserve( kGcmNonceLength + roomKey.size() + kGcmTagLength );
    envelope.append( reinterpret_cast<const char *>( nonce ), kGcmNonceLength );

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if ( ctx == nullptr )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    int           outLen = 0;
    int           ok     = 0;
    std::string   ciphertext( roomKey.size(), '\0' );
    unsigned char tag[ kGcmTagLength ] = { 0 };

    if ( EVP_EncryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, memberKey.data(), nonce ) == 1
         && EVP_EncryptUpdate( ctx,
                               reinterpret_cast<unsigned char *>( ciphertext.data() ),
                               &outLen,
                               roomKey.data(),
                               static_cast<int>( roomKey.size() ) )
                == 1
         && EVP_EncryptFinal_ex( ctx, nullptr, &outLen ) == 1 )
    {
        ok = EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_GET_TAG,
                                  static_cast<int>( kGcmTagLength ), tag );
    }

    EVP_CIPHER_CTX_free( ctx ); // always

    if ( ok != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    envelope.append( ciphertext );
    envelope.append( reinterpret_cast<const char *>( tag ), kGcmTagLength );
    return envelope;
}

outcome::result<std::vector<unsigned char>> UnwrapRoomKey(
    const std::vector<unsigned char> &memberKey, const std::string &envelope )
{
    if ( memberKey.size() != kRoomKeyLengthBytes
         || envelope.size() < kGcmNonceLength + kGcmTagLength )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    const unsigned char *nonce      = reinterpret_cast<const unsigned char *>( envelope.data() );
    const size_t         cipherLen  = envelope.size() - kGcmNonceLength - kGcmTagLength;
    const unsigned char *ciphertext = nonce + kGcmNonceLength;
    const unsigned char *tag        = ciphertext + cipherLen;

    if ( cipherLen != kRoomKeyLengthBytes )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if ( ctx == nullptr )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    int                        outLen = 0;
    int                        ok     = 0;
    std::vector<unsigned char> plaintext( cipherLen, 0 );

    if ( EVP_DecryptInit_ex( ctx, EVP_aes_256_gcm(), nullptr, memberKey.data(), nonce ) == 1
         && EVP_DecryptUpdate( ctx,
                               plaintext.data(),
                               &outLen,
                               ciphertext,
                               static_cast<int>( cipherLen ) )
                == 1
         && EVP_CIPHER_CTX_ctrl( ctx, EVP_CTRL_GCM_SET_TAG,
                                 static_cast<int>( kGcmTagLength ),
                                 const_cast<unsigned char *>( tag ) )
                == 1 )
    {
        int finalLen = 0;
        ok = EVP_DecryptFinal_ex( ctx, nullptr, &finalLen );
    }

    EVP_CIPHER_CTX_free( ctx ); // always

    if ( ok != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    return plaintext;
}

std::vector<unsigned char> InviteAuthMarker( const std::string &scopeId,
                                             const std::string &inviteId )
{
    // Bind BOTH ids, each NUL-separated from the domain prefix and from each
    // other, so an authenticator lifted from one invite cannot verify another
    // (T-04-09-07).
    std::string bound{ kInviteAuthMarkerPrefix };
    bound.push_back( '\0' );
    bound.append( scopeId );
    bound.push_back( '\0' );
    bound.append( inviteId );

    std::vector<unsigned char> marker( kInviteAuthMarkerLengthBytes, 0 );
    unsigned int               outLen = 0;
    if ( EVP_Digest( bound.data(), bound.size(), marker.data(), &outLen,
                     EVP_sha256(), nullptr )
             != 1
         || outLen != kInviteAuthMarkerLengthBytes )
    {
        return {}; // documented empty-on-failure contract (header)
    }
    return marker;
}

outcome::result<std::string> MintInviteAuthenticator(
    const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
    const std::string &inviteId )
{
    if ( inviteKey.size() != kRoomKeyLengthBytes )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    auto authKey = DeriveHkdfKey(
        inviteKey.data(), static_cast<int>( inviteKey.size() ),
        kInviteAuthHkdfSalt, sizeof( kInviteAuthHkdfSalt ) - 1,
        kInviteAuthHkdfInfo, sizeof( kInviteAuthHkdfInfo ) - 1,
        kRoomKeyLengthBytes );
    if ( !authKey.has_value() )
    {
        return authKey.error();
    }
    const std::vector<unsigned char> marker = InviteAuthMarker( scopeId, inviteId );
    if ( marker.size() != kInviteAuthMarkerLengthBytes )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    return WrapRoomKey( authKey.value(), marker );
}

outcome::result<bool> VerifyInviteAuthenticator(
    const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
    const std::string &inviteId, const std::string &authenticator )
{
    if ( inviteKey.empty() )
    {
        return outcome::success( false ); // a malformed bearer key never verifies
    }
    auto authKey = DeriveHkdfKey(
        inviteKey.data(), static_cast<int>( inviteKey.size() ),
        kInviteAuthHkdfSalt, sizeof( kInviteAuthHkdfSalt ) - 1,
        kInviteAuthHkdfInfo, sizeof( kInviteAuthHkdfInfo ) - 1,
        kRoomKeyLengthBytes );
    if ( !authKey.has_value() )
    {
        return authKey.error();
    }
    const auto unwrapped = UnwrapRoomKey( authKey.value(), authenticator );
    if ( !unwrapped.has_value() )
    {
        return outcome::success( false ); // wrong key, tampering, truncation
    }
    const std::vector<unsigned char> expected = InviteAuthMarker( scopeId, inviteId );
    if ( expected.size() != unwrapped.value().size() )
    {
        return outcome::success( false );
    }
    if ( CRYPTO_memcmp( expected.data(), unwrapped.value().data(),
                        expected.size() )
         != 0 )
    {
        return outcome::success( false );
    }
    return outcome::success( true );
}

} // namespace gcs::crypto
