/**
 * @file       test_gcs_crypto.cpp
 * @brief      gcs::crypto adapter unit tests (D-08): HKDF room-key derivation,
 *             AES-256-GCM round-trip, envelope shape, nonce uniqueness, and
 *             wrong-key/tamper/truncation rejection.
 * @details    Pure unit tests — no pubsub/DB fixture, no threads, no sleeps.
 *             The adapter is stateless, so every test calls the public
 *             functions directly and asserts the pinned envelope contract
 *             (nonce(12) || ciphertext || tag(16), room key =
 *             HKDF-SHA256(room_topic)). Every length assertion uses the
 *             kGcmNonceLength / kGcmTagLength / kRoomKeyLengthBytes constants
 *             — no magic 12/16/28/32 literals.
 * @date       2026-09-23
 */

#include "lib/gcs_crypto.hpp"

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace gcs::crypto
{
namespace
{
    /**
     * @brief Builds a binary-safe plaintext exercising the size-explicit
     *        contract: an embedded NUL byte plus high-bit bytes (0x80-0xFF)
     *        that C-string APIs would truncate or mangle.
     *
     * @return The binary plaintext payload.
     */
    std::string MakeBinaryPlaintext()
    {
        std::string plain;
        plain.reserve( 8 );
        plain.push_back( 'a' );
        plain.push_back( '\0' );                       // embedded NUL
        plain.push_back( 'b' );
        plain.push_back( static_cast<char>( 0x80 ) );  // high-bit byte
        plain.push_back( static_cast<char>( 0xFF ) );  // high-bit byte
        plain.push_back( 'c' );
        return plain;
    }
} // namespace

/**
 * @brief HKDF: DeriveRoomKey succeeds with a 32-byte key, is deterministic for
 *        one topic, and differs across topics.
 */
TEST( GcsCryptoTests, DeriveRoomKeyIsDeterministicAndRoomDistinct )
{
    const std::string roomA = "roomA";
    const std::string roomB = "roomB";

    auto keyA1 = DeriveRoomKey( roomA );
    ASSERT_TRUE( keyA1.has_value() );
    ASSERT_EQ( keyA1.value().size(), kRoomKeyLengthBytes );

    auto keyA2 = DeriveRoomKey( roomA );
    ASSERT_TRUE( keyA2.has_value() );
    EXPECT_EQ( keyA1.value(), keyA2.value() ); // same topic -> identical key

    auto keyB = DeriveRoomKey( roomB );
    ASSERT_TRUE( keyB.has_value() );
    ASSERT_EQ( keyB.value().size(), kRoomKeyLengthBytes );
    EXPECT_NE( keyA1.value(), keyB.value() ); // different topic -> different key
}

/**
 * @brief Round-trip is binary-safe: a plaintext carrying an embedded NUL and
 *        high-bit bytes decrypts back to the exact original bytes.
 */
TEST( GcsCryptoTests, RoundTripPreservesBinaryPlaintext )
{
    const std::string plain = MakeBinaryPlaintext();

    auto encrypted = EncryptPayload( "roomA", plain );
    ASSERT_TRUE( encrypted.has_value() );

    auto decrypted = DecryptPayload( "roomA", encrypted.value() );
    ASSERT_TRUE( decrypted.has_value() );
    EXPECT_EQ( decrypted.value(), plain );
}

/**
 * @brief Envelope shape: size == plaintext + nonce + tag, and the envelope
 *        never contains the plaintext substring (ciphertext at rest/wire).
 */
TEST( GcsCryptoTests, EnvelopeHasNonceCiphertextTagLayout )
{
    const std::string plain = "hello";

    auto encrypted = EncryptPayload( "roomA", plain );
    ASSERT_TRUE( encrypted.has_value() );

    EXPECT_EQ( encrypted.value().size(),
               plain.size() + kGcmNonceLength + kGcmTagLength );
    EXPECT_EQ( encrypted.value().find( plain ), std::string::npos );
}

/**
 * @brief Nonce uniqueness: two encryptions of the same plaintext under the
 *        same room produce different envelopes (fresh 12-byte RAND_bytes nonce
 *        per message), and both decrypt back to the plaintext.
 */
TEST( GcsCryptoTests, FreshNoncePerMessage )
{
    const std::string plain = "same-plaintext";

    auto first = EncryptPayload( "roomA", plain );
    ASSERT_TRUE( first.has_value() );
    auto second = EncryptPayload( "roomA", plain );
    ASSERT_TRUE( second.has_value() );

    EXPECT_NE( first.value(), second.value() );

    auto decryptedFirst = DecryptPayload( "roomA", first.value() );
    ASSERT_TRUE( decryptedFirst.has_value() );
    EXPECT_EQ( decryptedFirst.value(), plain );

    auto decryptedSecond = DecryptPayload( "roomA", second.value() );
    ASSERT_TRUE( decryptedSecond.has_value() );
    EXPECT_EQ( decryptedSecond.value(), plain );
}

/**
 * @brief Wrong key: decrypting an envelope with a different room's key fails
 *        (GCM tag mismatch).
 */
TEST( GcsCryptoTests, WrongRoomKeyFailsDecryption )
{
    auto encrypted = EncryptPayload( "roomA", "secret" );
    ASSERT_TRUE( encrypted.has_value() );

    EXPECT_FALSE( DecryptPayload( "roomB", encrypted.value() ).has_value() );
}

/**
 * @brief Tamper: flipping a single byte in the nonce, ciphertext, or tag region
 *        makes decryption fail (GCM integrity).
 */
TEST( GcsCryptoTests, TamperedEnvelopeFailsDecryption )
{
    const std::string plain = "tamper-me";
    auto             encrypted = EncryptPayload( "roomA", plain );
    ASSERT_TRUE( encrypted.has_value() );
    const std::string envelope = encrypted.value();

    // Nonce region.
    std::string tamperedNonce = envelope;
    tamperedNonce[ 0 ] = static_cast<char>( tamperedNonce[ 0 ] ^ 0x01 );
    EXPECT_FALSE( DecryptPayload( "roomA", tamperedNonce ).has_value() );

    // Ciphertext region.
    std::string tamperedCiphertext = envelope;
    const size_t cipherIndex = kGcmNonceLength;
    tamperedCiphertext[ cipherIndex ] =
        static_cast<char>( tamperedCiphertext[ cipherIndex ] ^ 0x01 );
    EXPECT_FALSE( DecryptPayload( "roomA", tamperedCiphertext ).has_value() );

    // Tag region.
    std::string tamperedTag = envelope;
    const size_t tagIndex = envelope.size() - kGcmTagLength;
    tamperedTag[ tagIndex ] = static_cast<char>( tamperedTag[ tagIndex ] ^ 0x01 );
    EXPECT_FALSE( DecryptPayload( "roomA", tamperedTag ).has_value() );
}

/**
 * @brief Truncation: an envelope shorter than nonce + tag has no complete frame
 *        and fails immediately (before any EVP work).
 */
TEST( GcsCryptoTests, TruncatedEnvelopeFailsImmediately )
{
    const std::string tooShort( kGcmNonceLength + kGcmTagLength - 1, 'x' );
    EXPECT_FALSE( DecryptPayload( "roomA", tooShort ).has_value() );
}

} // namespace gcs::crypto
