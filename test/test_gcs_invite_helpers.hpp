/**
 * @file       test_gcs_invite_helpers.hpp
 * @brief      Shared invite-test helpers: the canonical issuer payload builder.
 * @details    Mirrors `InviteIssuerPayload` in gcs_membership.cpp so the
 *             membership unit suite and the multinode suite re-sign invite
 *             records the SAME way. This is the ONE test-side implementation
 *             (invariant 11) — never copy the builder into either test file,
 *             and never use it as a production implementation.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_TEST_INVITE_HELPERS_HPP
#define GCS_TEST_INVITE_HELPERS_HPP

#include <string>

#include "proto/gcs_chat.pb.h"

namespace gcs::test
{
    /// Domain separator mirroring the production kInviteIssuerDomain.
    inline constexpr const char *kInviteIssuerDomain = "gcs-invite-issuer-v1";

    /**
     * @brief Build the canonical bytes an invite issuer signs.
     *
     * The invite id is explicit because it is the record's KEY (not a record
     * field); the issuer_signature field is cleared because a signature cannot
     * cover itself.
     *
     * @param[in] inviteId The invite id (the record key's final segment).
     * @param[in] record   The invite record to canonicalize.
     * @return domain + '\n' + inviteId + '\n' + serialized record bytes.
     */
    inline std::string BuildInviteIssuerPayload( const std::string &inviteId,
                                                 const chat::InviteRecord &record )
    {
        chat::InviteRecord canonical = record;
        canonical.clear_issuer_signature();
        return std::string( kInviteIssuerDomain ) + '\n' + inviteId + '\n'
               + canonical.SerializeAsString();
    }
} // namespace gcs::test

#endif // GCS_TEST_INVITE_HELPERS_HPP
