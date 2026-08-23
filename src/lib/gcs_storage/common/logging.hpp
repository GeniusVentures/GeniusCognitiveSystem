/**
 * @file       logging.hpp
 * @brief      Logging facade — wraps spdlog directly
 *             (vendored from GNUS-NEO-SWARM common/ — GCS-owned copy)
 */

#ifndef GCS_STORAGE_COMMON_LOGGING_HPP
#define GCS_STORAGE_COMMON_LOGGING_HPP

#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

namespace sgns::gcs {
/// Logger sgns::base::Logger convention
using Logger = std::shared_ptr<spdlog::logger>;

/**
 * @brief Create a named logger for a GCS component.
 *
 * @param tag  Component name shown in log output (e.g. "GlobalDb", "Ffi").
 * @return     Logger instance.
 */
inline Logger CreateLogger(const std::string &tag) {
  const std::string name = "Gcs/" + tag;
  auto existing = spdlog::get(name);
  if (existing) {
    return existing;
  }
  auto logger = spdlog::stdout_color_mt(name);
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
  return logger;
}

} // namespace sgns::gcs

#endif // GCS_STORAGE_COMMON_LOGGING_HPP
