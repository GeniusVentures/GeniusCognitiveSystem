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

#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>

#include "gcs_storage/common/error.hpp" // sgns::gcs::Error (outcome failure domain)

namespace gcs::crypto
{

outcome::result<std::vector<unsigned char>> DeriveRoomKey( const std::string &roomTopic )
{
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id( EVP_PKEY_HKDF, nullptr );
    if ( pctx == nullptr )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    std::vector<unsigned char> key( kRoomKeyLengthBytes, 0 );
    size_t                     outLen = key.size();
    const int ok =
        ( EVP_PKEY_derive_init( pctx ) == 1 )
        && ( EVP_PKEY_CTX_set_hkdf_md( pctx, EVP_sha256() ) == 1 )
        && ( EVP_PKEY_CTX_set1_hkdf_salt(
                 pctx,
                 reinterpret_cast<const unsigned char *>( kMessagesHkdfSalt ),
                 static_cast<int>( sizeof( kMessagesHkdfSalt ) - 1 ) )
             == 1 )
        && ( EVP_PKEY_CTX_set1_hkdf_key(
                 pctx,
                 reinterpret_cast<const unsigned char *>( roomTopic.data() ),
                 static_cast<int>( roomTopic.size() ) )
             == 1 )
        && ( EVP_PKEY_CTX_add1_hkdf_info(
                 pctx,
                 reinterpret_cast<const unsigned char *>( kMessagesHkdfInfo ),
                 static_cast<int>( sizeof( kMessagesHkdfInfo ) - 1 ) )
             == 1 )
        && ( EVP_PKEY_derive( pctx, key.data(), &outLen ) == 1 )
        && ( outLen == kRoomKeyLengthBytes );

    EVP_PKEY_CTX_free( pctx ); // always

    if ( ok != 1 )
    {
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    return key;
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

} // namespace gcs::crypto
