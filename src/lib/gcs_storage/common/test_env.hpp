/**
 * @file       test_env.hpp
 * @brief      Test-environment helpers for GCS components that boot the
 *             GeniusSDK node.
 * @details    Node boot creates a GeniusAccount, whose default secure storage
 *             is OS-backed (Apple Keychain / libsecret / Windows DPAPI-adjacent
 *             file backends). On CI the OS keychain is unreliable: the macOS
 *             runner's login keychain is session-scoped and intermittently
 *             unavailable, so SecItemAdd fails, account creation dies with
 *             "Failed to generate Genius address from private key", and every
 *             node-booting test fails to boot (CI runs 35904291986 /
 *             35918456744 — the option-C tests SKIP, test_gcs_ffi hard-fails).
 *
 *             SuperGenius's own suite never touches the OS keychain: 38 test
 *             files call GeniusAccount::SetSecureStorageFactory with
 *             MemorySecureStorage ("inject in-memory secure storage to avoid
 *             OS keychain prompts during tests"). This header is that exact
 *             house pattern, exposed for GCS tests.
 *
 *             IMAGE RULE: GeniusAccount::SetSecureStorageFactory sets a
 *             static in the TRANSLATION SCOPE THAT LINKS sgns_genius_account
 *             into the calling image. The node must boot in the SAME image
 *             where the factory is set:
 *               - tests that boot the SDK from their own static chain call
 *                 UseInMemorySecureStorage() directly (this header);
 *               - tests that boot the node through gcs_ffi's embedded boot
 *                 (gcs_init boots it inside the DLL) must use the DLL's
 *                 exported gcs_use_test_secure_storage() so the factory is
 *                 set on the DLL's copy — a call from the exe would set the
 *                 exe's own static and never reach the embedded boot.
 *
 *             Volatile-by-design: wallet/secret data lives only for the
 *             process lifetime (MemorySecureStorage keeps an in-process
 *                 map). Test-only; never link into shipping binaries.
 * @date       2026-09-23
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_STORAGE_TEST_ENV_HPP
#define GCS_STORAGE_TEST_ENV_HPP

#include "account/GeniusAccount.hpp"
#include "local_secure_storage/impl/MemorySecureStorage.hpp"

namespace gcs_storage::test
{
    /**
     * @brief   Redirects GeniusAccount secure storage to an in-memory backend.
     * @details SuperGenius house pattern (38 upstream test files): replaces
     *          the OS keychain backend with MemorySecureStorage so node boot
     *          never prompts and never fails on a session-scoped CI keychain.
     *          Call BEFORE the first GeniusSDKInit/GcsGlobalDb::Initialize in
     *          the calling image. Idempotent: repeated calls reinstall the
     *          same factory lambda.
     *
     *          Must be called from the image that boots the node — see the
     *          IMAGE RULE in the file header.
     */
    inline void UseInMemorySecureStorage()
    {
        sgns::GeniusAccount::SetSecureStorageFactory(
            []( const std::string &identifier ) -> std::shared_ptr<sgns::ISecureStorage>
            { return std::make_shared<sgns::MemorySecureStorage>( identifier ); } );
    }
} // namespace gcs_storage::test

#endif // GCS_STORAGE_TEST_ENV_HPP
