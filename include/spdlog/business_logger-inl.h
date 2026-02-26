// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/business_logger.h>
#endif

#include <spdlog/sinks/rotating_file_sink.h>

namespace spdlog {

SPDLOG_INLINE BusinessLoggerManager& BusinessLoggerManager::instance() {
    static BusinessLoggerManager instance;
    return instance;
}

SPDLOG_INLINE void BusinessLoggerManager::init(const std::string& log_dir,
                                                size_t max_file_size,
                                                size_t max_files) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_dir_ = log_dir;
    max_file_size_ = max_file_size;
    max_files_ = max_files;
    initialized_ = true;
}

SPDLOG_INLINE void BusinessLoggerManager::log(BusinessType type, level::level_enum lvl, const std::string& msg) {
    auto logger = get_logger(type);
    if (logger) {
        logger->log(lvl, msg);
    }
}

SPDLOG_INLINE void BusinessLoggerManager::trace(BusinessType type, const std::string& msg) {
    log(type, level::trace, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::debug(BusinessType type, const std::string& msg) {
    log(type, level::debug, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::info(BusinessType type, const std::string& msg) {
    log(type, level::info, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::warn(BusinessType type, const std::string& msg) {
    log(type, level::warn, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::error(BusinessType type, const std::string& msg) {
    log(type, level::err, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::critical(BusinessType type, const std::string& msg) {
    log(type, level::critical, msg);
}

SPDLOG_INLINE void BusinessLoggerManager::set_level(BusinessType type, level::level_enum lvl) {
    auto logger = get_logger(type);
    if (logger) {
        logger->set_level(lvl);
    }
}

SPDLOG_INLINE void BusinessLoggerManager::set_level_for_all(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        if (pair.second) {
            pair.second->set_level(lvl);
        }
    }
}

SPDLOG_INLINE void BusinessLoggerManager::flush_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        if (pair.second) {
            pair.second->flush();
        }
    }
}

SPDLOG_INLINE std::shared_ptr<logger> BusinessLoggerManager::get_logger(BusinessType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    create_logger(type);
    return loggers_[type];
}

SPDLOG_INLINE std::string BusinessLoggerManager::business_type_to_string(BusinessType type) const {
    switch (type) {
        case BusinessType::RecordScreen:
            return "record_screen";
        case BusinessType::OpenDesktop:
            return "open_desktop";
        case BusinessType::RecordKeyboard:
            return "record_keyboard";
        case BusinessType::RecordAudio:
            return "record_audio";
        default:
            return "unknown";
    }
}

SPDLOG_INLINE BusinessType BusinessLoggerManager::string_to_business_type(const std::string& str) const {
    if (str == "record_screen") return BusinessType::RecordScreen;
    if (str == "open_desktop") return BusinessType::OpenDesktop;
    if (str == "record_keyboard") return BusinessType::RecordKeyboard;
    if (str == "record_audio") return BusinessType::RecordAudio;
    return BusinessType::Unknown;
}

SPDLOG_INLINE void BusinessLoggerManager::create_logger(BusinessType type) {
    if (!initialized_) {
        return;
    }
    
    std::string logger_name = business_type_to_string(type);
    std::string file_path = log_dir_ + "/" + logger_name + ".log";
    
    auto rotating_sink = std::make_shared<sinks::rotating_file_sink_mt>(
        file_path, max_file_size_, max_files_);
    
    auto logger = std::make_shared<spdlog::logger>(logger_name, rotating_sink);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    logger->set_level(level::debug);
    
    loggers_[type] = logger;
}

}  // namespace spdlog
