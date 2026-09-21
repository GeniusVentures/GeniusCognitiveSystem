/**
 * @file       logging.hpp
 * @brief      Logging facade — wraps spdlog directly
 *             (vendored from GNUS-NEO-SWARM common/ — GCS-owned copy;
 *             extended 2026-09-19: GCS loggers mirror to a rotating file
 *             under a caller-set base path — file logs are the post-hoc
 *             diagnostic surface for packaged apps, per GeniusWallet)
 */

#ifndef GCS_STORAGE_COMMON_LOGGING_HPP
#define GCS_STORAGE_COMMON_LOGGING_HPP

#include <filesystem>
#include <memory>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace sgns::gcs {
/// Logger sgns::base::Logger convention
using Logger = std::shared_ptr<spdlog::logger>;

/// Log file name written under the GCS file-log base path.
inline constexpr const char *kGcsLogFileName = "gcs_chat.log";
/// Rotating log file size cap in bytes (bounded disk growth in sandboxes).
inline constexpr size_t kGcsLogMaxBytes = 5 * 1024 * 1024;
/// Maximum rotated files kept beside the active log.
inline constexpr size_t kGcsLogMaxFiles = 3;

namespace detail {
/// The shared rotating file sink (null until SetGcsFileLogBasePath runs;
/// written only from gcs_init under the FFI mutex, read afterwards).
inline std::shared_ptr<spdlog::sinks::sink> &SharedFileSink() {
  static std::shared_ptr<spdlog::sinks::sink> sink;
  return sink;
}
} // namespace detail

/**
 * @brief Direct GCS component logs to a rotating gcs_chat.log under
 *        base_path, in addition to the console. Call before the first GCS
 *        logger is created (gcs_init does so); the first base path wins for
 *        the process. An unwritable path falls back to console-only and
 *        never takes the session down.
 *
 * @param base_path  Directory the log file is written to.
 */
inline void SetGcsFileLogBasePath(const std::filesystem::path &base_path) {
  auto &slot = detail::SharedFileSink();
  if (slot) {
    return; // already configured — one file per process, first path wins
  }
  try {
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        (base_path / kGcsLogFileName).string(), kGcsLogMaxBytes, kGcsLogMaxFiles);
    file_sink->set_level(spdlog::level::debug);
    slot = std::move(file_sink);
  } catch (const std::exception &error) {
    spdlog::error("gcs logging: file sink unavailable ({}), console only",
                  error.what());
  }
}

/**
 * @brief Create a named logger for a GCS component. Mirrors to the shared
 *        rotating file sink when one has been configured.
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
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::move(console_sink));
  if (auto file_sink = detail::SharedFileSink()) {
    sinks.push_back(std::move(file_sink));
  }
  auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::debug);
  logger->flush_on(spdlog::level::debug); // low volume — keep files crash-current
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
  spdlog::register_logger(logger);
  return logger;
}

/**
 * @brief Re-point spdlog's default logger (bare spdlog:: calls) at the
 *        console plus the shared rotating file sink, so unnamed GCS
 *        component logs are filed too. No-op when no file sink is set.
 */
inline void ApplyFileSinkToDefaultLogger() {
  auto file_sink = detail::SharedFileSink();
  if (!file_sink) {
    return;
  }
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::info);
  auto logger = std::make_shared<spdlog::logger>(
      "gcs", spdlog::sinks_init_list{std::move(console_sink), std::move(file_sink)});
  logger->set_level(spdlog::level::debug);
  logger->flush_on(spdlog::level::debug);
  logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v");
  spdlog::set_default_logger(logger);
}

} // namespace sgns::gcs

#endif // GCS_STORAGE_COMMON_LOGGING_HPP
