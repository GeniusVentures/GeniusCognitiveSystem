/**
 * @file       test_gcs_member_helpers.hpp
 * @brief      Shared member-test helpers: the canonical authority and leave
 *             payload builders plus the seam double's signature tokens.
 * @details    Mirrors `MemberIssuerPayload` and `MemberLeavePayload` in
 *             gcs_membership.cpp so the membership unit suite signs member
 *             records the SAME way the production verifier reads them. This is
 *             the ONE test-side implementation (invariant 11) — never copy a
 *             builder into a test file, and never use it as a production
 *             implementation.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_TEST_MEMBER_HELPERS_HPP
#define GCS_TEST_MEMBER_HELPERS_HPP

#include <string>

#include "lib/gcs_crypto.hpp"
#include "proto/gcs_chat.pb.h"

namespace gcs::test
{
    /// Domain separator mirroring the production kMemberIssuerDomain.
    inline constexpr const char *kMemberIssuerDomain = "gcs-member-issuer-v1";

    /// Domain separator mirroring the production kMemberLeaveDomain.
    inline constexpr const char *kMemberLeaveDomain = "gcs-member-leave-v1";

    /**
     * @brief Build the canonical bytes a member authority write signs.
     *
     * Mirrors production: the scope id and address are explicit because they
     * are the record key, and the self-signature, the subject leave signature,
     * bearer room-key material, and the subject-owned leave flag are cleared
     * because they are never authority. Timestamps and prior_role are kept.
     *
     * @param[in] scopeId The scope the record belongs to.
     * @param[in] address The wallet the record is stored under.
     * @param[in] record  The member record to canonicalize.
     * @return domain + '\n' + scopeId + '\n' + address + '\n' + record bytes.
     */
    inline std::string BuildMemberIssuerPayload( const std::string        &scopeId,
                                                 const std::string        &address,
                                                 const chat::MemberRecord &record )
    {
        chat::MemberRecord canonical = record;
        canonical.clear_record_signature();
        canonical.clear_leave_signature();
        canonical.clear_wrapped_room_key();
        canonical.clear_explicit_leave();
        return std::string( kMemberIssuerDomain ) + '\n' + scopeId + '\n' + address + '\n' +
               canonical.SerializeAsString();
    }

    /**
     * @brief Build the canonical bytes a member signs to leave a scope.
     *
     * The leave attestation is deliberately independent of the record bytes: a
     * later authority rewrite cannot invalidate the subject's leave.
     *
     * @param[in] scopeId The scope being left.
     * @param[in] address The leaving subject's wallet address.
     * @return domain + '\n' + scopeId + '\n' + address.
     */
    inline std::string BuildMemberLeavePayload( const std::string &scopeId, const std::string &address )
    {
        return std::string( kMemberLeaveDomain ) + '\n' + scopeId + '\n' + address;
    }

    /**
     * @brief The HEX-ENCODED authority signature the MakeSeam double emits.
     *
     * The double's token embeds the payload, which can contain arbitrary
     * bytes, so it is hex-encoded exactly like production stores it in the
     * proto3 `record_signature` string field.
     *
     * @param[in] issuer  The claimed issuer address.
     * @param[in] scopeId The scope the record belongs to.
     * @param[in] address The wallet the record is stored under.
     * @param[in] record  The member record being signed.
     * @return The double's signature token, hex-encoded.
     */
    inline std::string TestMemberSignatureHex( const std::string        &issuer,
                                               const std::string        &scopeId,
                                               const std::string        &address,
                                               const chat::MemberRecord &record )
    {
        return crypto::HexEncode( std::string( "testsig:" ) + issuer + ":" +
                                  BuildMemberIssuerPayload( scopeId, address, record ) );
    }

    /**
     * @brief The HEX-ENCODED subject leave signature the MakeSeam double emits.
     *
     * @param[in] subject The leaving wallet address (the required signer).
     * @param[in] scopeId The scope being left.
     * @param[in] address The wallet the record is stored under.
     * @return The double's signature token, hex-encoded.
     */
    inline std::string TestLeaveSignatureHex( const std::string &subject,
                                              const std::string &scopeId,
                                              const std::string &address )
    {
        return crypto::HexEncode( std::string( "testsig:" ) + subject + ":" +
                                  BuildMemberLeavePayload( scopeId, address ) );
    }
} // namespace gcs::test

#endif // GCS_TEST_MEMBER_HELPERS_HPP
