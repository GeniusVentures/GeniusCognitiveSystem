/**
 * @file       error.hpp
 * @brief      GCS-specific error codes and outcome::result alias.
 *             Only codes owned by GeniusCognitiveSystem live here —
 * engine/router/ memory errors remain in GNUS-NEO-SWARM's own common/error.hpp.
 *             Values 1..21 are reserved by the NEO-SWARM domain; GCS codes
 * start at 22 to keep the two domains collision-free.
 */

#ifndef GCS_STORAGE_COMMON_ERROR_HPP
#define GCS_STORAGE_COMMON_ERROR_HPP

#include <libp2p/outcome/outcome.hpp>

namespace sgns::gcs {
namespace outcome = libp2p::outcome;

// -----------------------------------------------------------------------
// GCS error codes (values continue the NEO-SWARM numbering — do not renumber)
// -----------------------------------------------------------------------
enum class Error : uint8_t {
  // GCS GlobalDB
  GcsDbError =
      22, ///< GCS GlobalDB operation failed (init, start, topic wiring)
  SdkNotInitialized =
      23, ///< GeniusSDKGetNode() returned nullptr — SDK init chain has not run
};

} // namespace sgns::gcs

// Register the error enum with Boost.Outcome so it can be used in
// outcome::result<>
OUTCOME_HPP_DECLARE_ERROR_2(sgns::gcs, Error)

#endif // GCS_STORAGE_COMMON_ERROR_HPP
