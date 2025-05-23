#pragma once

#include <boost/log/sources/severity_channel_logger.hpp>
#include <string>

#include "logger/logger_config.hpp"

namespace daily::logger {

// Concurrent (Thread-safe) logger type
using Logger = boost::log::sources::severity_channel_logger_mt<>;

/**
 * @brief Creates thread-safe severity channel logger
 * @note To control logging in terms of where log messages are forwarded(console/file), severity filter etc..., see
 *       functions createDefaultLoggingConfig and InitLogging. Usage example:
 *
 *       // Creates logging config
 *       auto logging_config = createDefaultLoggingConfig();
 *       logging_config.verbosity = logger::Verbosity::Error;
 *       logging_config.channels["SAMPLE_CHANNEL"] = logger::Verbosity::Error;
 *
 *       // Initializes logging according to the provided config
 *       InitLogging(logging_config);
 *
 *       addr_t node_addr;
 *
 *       // Creates error logger
 *       auto logger = createLogger(logger::Verbosity::Error, "SAMPLE_CHANNEL", node_addr)
 *
 *       LOG(logger) << "sample message";
 *
 * @note see macros LOG_OBJECTS_DEFINE, LOG_OBJECTS_CREATE, LOG
 * @param verboseLevel
 * @param _channel
 * @param node_id
 * @return Logger object
 */
Logger createLogger(Verbosity verboseLevel, const std::string& channel, const addr_t& node_id);

/**
 * @brief Creates default logging config
 *
 * @return logger::Config
 */
Config createDefaultLoggingConfig();

/**
 * @brief Initializes logging according to the provided logging_config
 *
 * @param logging_config
 * @param node_id
 */
void InitLogging(Config& logging_config, const addr_t& node_id);

}  // namespace daily::logger

#define LOG BOOST_LOG

#define LOG_OBJECTS_DEFINE                \
  mutable daily::logger::Logger log_si_; \
  mutable daily::logger::Logger log_er_; \
  mutable daily::logger::Logger log_wr_; \
  mutable daily::logger::Logger log_nf_; \
  mutable daily::logger::Logger log_dg_; \
  mutable daily::logger::Logger log_tr_;

#define LOG_OBJECTS_CREATE(channel)                                                               \
  log_si_ = daily::logger::createLogger(daily::logger::Verbosity::Silent, channel, node_addr);  \
  log_er_ = daily::logger::createLogger(daily::logger::Verbosity::Error, channel, node_addr);   \
  log_wr_ = daily::logger::createLogger(daily::logger::Verbosity::Warning, channel, node_addr); \
  log_nf_ = daily::logger::createLogger(daily::logger::Verbosity::Info, channel, node_addr);    \
  log_tr_ = daily::logger::createLogger(daily::logger::Verbosity::Trace, channel, node_addr);   \
  log_dg_ = daily::logger::createLogger(daily::logger::Verbosity::Debug, channel, node_addr);
