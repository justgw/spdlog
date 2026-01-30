// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// 业务日志管理器
// 支持根据不同业务逻辑生成不同的日志文件

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace spdlog {

enum class BusinessType {
    ScreenRecord,      
    DesktopOpen,       
    KeyboardRecord,    
    AudioRecord        
};

inline const char* business_type_to_string(BusinessType type) {
    switch (type) {
        case BusinessType::ScreenRecord:
            return "screen_record";
        case BusinessType::DesktopOpen:
            return "desktop_open";
        case BusinessType::KeyboardRecord:
            return "keyboard_record";
        case BusinessType::AudioRecord:
            return "audio_record";
        default:
            return "unknown";
    }
}

class business_logger_manager {
public:
    static business_logger_manager& instance() {
        static business_logger_manager instance;
        return instance;
    }

    void init(const std::string& log_dir = "logs") {
        std::lock_guard<std::mutex> lock(mutex_);
        log_dir_ = log_dir;
        max_file_size_ = 30 * 1024 * 1024;  
        max_files_ = 3;                      
        initialized_ = true;
    }

    void set_max_file_size(size_t size_bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        max_file_size_ = size_bytes;
    }

    void set_max_files(size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        max_files_ = count;
    }

    std::shared_ptr<logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!initialized_) {
            throw_spdlog_ex("business_logger_manager not initialized. Call init() first.");
        }

        auto it = loggers_.find(type);
        if (it != loggers_.end()) {
            return it->second;
        }

        return create_logger(type);
    }

    template <typename... Args>
    void log(BusinessType type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void trace(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::error, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::critical, fmt, std::forward<Args>(args)...);
    }

    void set_level(BusinessType type, level::level_enum lvl) {
        auto logger = get_logger(type);
        logger->set_level(lvl);
    }

    void set_all_levels(level::level_enum lvl) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->set_level(lvl);
        }
    }

    void flush_all() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->flush();
        }
    }

    void set_pattern(const std::string& pattern) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& pair : loggers_) {
            pair.second->set_pattern(pattern);
        }
        default_pattern_ = pattern;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        loggers_.clear();
        initialized_ = false;
    }

private:
    business_logger_manager() 
        : max_file_size_(30 * 1024 * 1024)
        , max_files_(3)
        , initialized_(false)
        , default_pattern_("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v")
    {}

    business_logger_manager(const business_logger_manager&) = delete;
    business_logger_manager& operator=(const business_logger_manager&) = delete;

    std::shared_ptr<logger> create_logger(BusinessType type) {
        std::string business_name = business_type_to_string(type);
        std::string logger_name = "business_" + business_name;
        std::string filename = log_dir_ + "/" + business_name + ".log";

        auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
            filename, max_file_size_, max_files_, false);

        auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
        logger->set_pattern(default_pattern_);
        logger->set_level(level::trace);
        logger->flush_on(level::err);

        loggers_[type] = logger;
        return logger;
    }

    std::mutex mutex_;
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::string log_dir_;
    size_t max_file_size_;
    size_t max_files_;
    bool initialized_;
    std::string default_pattern_;
};

}  // namespace spdlog
