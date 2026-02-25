// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/multi_business_logger.h>
#endif

#include <spdlog/common.h>
#include <spdlog/details/os.h>

#include <cstdlib>

namespace spdlog {

// MultiBusinessLogger 实现

inline MultiBusinessLogger::MultiBusinessLogger(const MultiBusinessLoggerConfig& config)
    : config_(config) {
    // 确保日志目录存在
    if (!config_.log_dir.empty()) {
        details::os::create_dir(config_.log_dir);
    }
}

inline std::shared_ptr<logger> MultiBusinessLogger::get_logger(BusinessType business_type) {
    std::string key = business_type_to_string(business_type);
    return get_logger(key);
}

inline std::shared_ptr<logger> MultiBusinessLogger::get_logger(const std::string& business_key) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loggers_.find(business_key);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    // 创建新的 logger
    return create_logger(business_key);
}

inline std::shared_ptr<logger> MultiBusinessLogger::create_logger(const std::string& business_key) {
    std::string filename = get_log_filename(business_key);
    
    auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
        filename,
        config_.max_file_size,
        config_.max_files,
        config_.rotate_on_open
    );
    
    auto new_logger = std::make_shared<logger>(business_key, sink);
    new_logger->set_level(config_.level);
    new_logger->set_pattern(config_.pattern);
    
    loggers_[business_key] = new_logger;
    return new_logger;
}

inline std::string MultiBusinessLogger::get_log_filename(const std::string& business_key) const {
    if (config_.log_dir.empty()) {
        return business_key + ".log";
    }
    return config_.log_dir + "/" + business_key + ".log";
}

inline bool MultiBusinessLogger::has_logger(BusinessType business_type) const {
    std::string key = business_type_to_string(business_type);
    return has_logger(key);
}

inline bool MultiBusinessLogger::has_logger(const std::string& business_key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return loggers_.find(business_key) != loggers_.end();
}

inline void MultiBusinessLogger::set_level(BusinessType business_type, level::level_enum lvl) {
    std::string key = business_type_to_string(business_type);
    set_level(key, lvl);
}

inline void MultiBusinessLogger::set_level(const std::string& business_key, level::level_enum lvl) {
    auto logger = get_logger(business_key);
    logger->set_level(lvl);
}

inline void MultiBusinessLogger::set_global_level(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_.level = lvl;
    for (auto& pair : loggers_) {
        pair.second->set_level(lvl);
    }
}

inline void MultiBusinessLogger::flush(BusinessType business_type) {
    std::string key = business_type_to_string(business_type);
    flush(key);
}

inline void MultiBusinessLogger::flush(const std::string& business_key) {
    auto logger = get_logger(business_key);
    logger->flush();
}

inline void MultiBusinessLogger::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        pair.second->flush();
    }
}

// MultiBusinessLoggerRegistry 实现

inline MultiBusinessLoggerRegistry& MultiBusinessLoggerRegistry::instance() {
    static MultiBusinessLoggerRegistry instance;
    return instance;
}

inline void MultiBusinessLoggerRegistry::initialize(const MultiBusinessLoggerConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!logger_) {
        logger_ = std::make_shared<MultiBusinessLogger>(config);
    }
}

inline std::shared_ptr<MultiBusinessLogger> MultiBusinessLoggerRegistry::get_logger() {
    std::lock_guard<std::mutex> lock(mutex_);
    return logger_;
}

inline bool MultiBusinessLoggerRegistry::is_initialized() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return logger_ != nullptr;
}

// 便捷函数实现

inline std::shared_ptr<MultiBusinessLogger> get_multi_business_logger() {
    return MultiBusinessLoggerRegistry::instance().get_logger();
}

inline void init_multi_business_logger(const MultiBusinessLoggerConfig& config) {
    MultiBusinessLoggerRegistry::instance().initialize(config);
}

} // namespace spdlog
