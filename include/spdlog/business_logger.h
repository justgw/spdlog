#pragma once

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace spdlog {

enum class BusinessType {
    RecordScreen,
    OpenDesktop,
    RecordKeyboard,
    RecordAudio,
    Unknown
};

class BusinessLoggerManager {
public:
    static BusinessLoggerManager& instance();

    void init(const std::string& log_dir,
              size_t max_file_size = 30 * 1024 * 1024,
              size_t max_files = 3);

    void log(BusinessType type, level::level_enum lvl, const std::string& msg);

    void trace(BusinessType type, const std::string& msg);
    void debug(BusinessType type, const std::string& msg);
    void info(BusinessType type, const std::string& msg);
    void warn(BusinessType type, const std::string& msg);
    void error(BusinessType type, const std::string& msg);
    void critical(BusinessType type, const std::string& msg);

    template <typename... Args>
    void log(BusinessType type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
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
        log(type, level::err, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(BusinessType type, format_string_t<Args...> fmt, Args&&... args) {
        log(type, level::critical, fmt, std::forward<Args>(args)...);
    }

    void set_level(BusinessType type, level::level_enum lvl);
    void set_level_for_all(level::level_enum lvl);
    void flush_all();

    std::shared_ptr<logger> get_logger(BusinessType type);
    std::string business_type_to_string(BusinessType type) const;
    BusinessType string_to_business_type(const std::string& str) const;

private:
    BusinessLoggerManager() = default;
    ~BusinessLoggerManager() = default;
    BusinessLoggerManager(const BusinessLoggerManager&) = delete;
    BusinessLoggerManager& operator=(const BusinessLoggerManager&) = delete;

    void create_logger(BusinessType type);

    std::string log_dir_;
    size_t max_file_size_;
    size_t max_files_;
    std::mutex mutex_;
    std::unordered_map<BusinessType, std::shared_ptr<logger>> loggers_;
    bool initialized_ = false;
};

}  // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#include "business_logger-inl.h"
#endif
