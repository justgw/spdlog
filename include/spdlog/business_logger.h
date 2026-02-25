#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace spdlog {

enum class BusinessType {
    ScreenRecording,
    DesktopOpen,
    KeyboardRecording,
    AudioRecording
};

class BusinessLogger {
public:
    static BusinessLogger& instance() {
        static BusinessLogger instance;
        return instance;
    }

    void init(const std::string& log_dir = "logs") {
        std::lock_guard<std::mutex> lock(mutex_);
        log_dir_ = log_dir;
        init_logger(BusinessType::ScreenRecording, "screen_recording");
        init_logger(BusinessType::DesktopOpen, "desktop_open");
        init_logger(BusinessType::KeyboardRecording, "keyboard_recording");
        init_logger(BusinessType::AudioRecording, "audio_recording");
    }

    template <typename... Args>
    void log(BusinessType type, spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger_internal(type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    void trace(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_error(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(BusinessType type, spdlog::format_string_t<Args...> fmt, Args&&... args) {
        log(type, spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

    std::shared_ptr<logger> get_logger(BusinessType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        return get_logger_internal(type);
    }

    void set_level(spdlog::level::level_enum lvl) {
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
    }

    static std::string business_type_to_string(BusinessType type) {
        switch (type) {
            case BusinessType::ScreenRecording:
                return "ScreenRecording";
            case BusinessType::DesktopOpen:
                return "DesktopOpen";
            case BusinessType::KeyboardRecording:
                return "KeyboardRecording";
            case BusinessType::AudioRecording:
                return "AudioRecording";
            default:
                return "Unknown";
        }
    }

private:
    BusinessLogger() = default;
    ~BusinessLogger() = default;
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    std::shared_ptr<logger> get_logger_internal(BusinessType type) {
        auto it = loggers_.find(static_cast<int>(type));
        if (it != loggers_.end()) {
            return it->second;
        }
        return nullptr;
    }

    void init_logger(BusinessType type, const std::string& name) {
        std::string filename = log_dir_ + "/" + name + ".log";
        constexpr size_t max_size = 30 * 1024 * 1024;
        constexpr size_t max_files = 3;
        
        auto logger = rotating_logger_mt(name, filename, max_size, max_files, true);
        logger->set_level(spdlog::level::trace);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
        logger->flush_on(spdlog::level::err);
        loggers_[static_cast<int>(type)] = logger;
    }

    std::mutex mutex_;
    std::string log_dir_ = "logs";
    std::map<int, std::shared_ptr<logger>> loggers_;
};

}
