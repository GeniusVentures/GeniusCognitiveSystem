/**
 * @file       error.cpp
 * @brief      Boost.Outcome error category registration for the GCS error
 * domain.
 */

#include "error.hpp"

OUTCOME_CPP_DEFINE_CATEGORY_3(sgns::gcs, Error, e) {
  using E = sgns::gcs::Error;
  switch (e) {
  case E::GcsDbError:
    return "GCS GlobalDB operation failed";
  case E::SdkNotInitialized:
    return "GeniusSDK not initialized";
  }
  return "Unknown error";
}
