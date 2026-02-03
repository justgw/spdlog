// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/logger.h>

#include <string>
#include <memory>
#include <map>
#include <mutex>

namespace spdlog {

// 业务类型枚举
enum class BusinessType {
    ScreenRecord,   // 录制屏幕
    DesktopOpen,    // 打开桌面
    KeyboardRecord, // 录制键盘
    SoundRecord     // 录制声音
};

// 将业务类型转换为字符串
inline std::string business_type_to_string(BusinessType type) {
    switch (type) {
        case BusinessType::ScreenRecord:   return "screen_record";
        case BusinessType::DesktopOpen:    return "desktop_open";
        case BusinessType::KeyboardRecord: return "keyboard_record";
        case BusinessType::SoundRecord:    return "sound_record";
        default: return "unknown";
    }
}

// 业务日志管理器类
class BusinessLogger {
public:
    // 默认配置：最大文件大小30M，最多保留3个文件
    static constexpr size_t DEFAULT_MAX_FILE_SIZE = 30 * 1024 * 1024;  // 30MB
    static constexpr size_t DEFAULT_MAX_FILES = 3;

    // 构造函数
    explicit BusinessLogger(const std::string& log_dir = "logs",
                           size_t max_file_size = DEFAULT_MAX_FILE_SIZE,
                           size_t max_files = DEFAULT_MAX_FILES);

    // 析构函数
    ~BusinessLogger() = default;

    // 禁止拷贝和赋值
    BusinessLogger(const BusinessLogger&) = delete;
    BusinessLogger& operator=(const BusinessLogger&) = delete;

    // 获取单例实例
    static std::shared_ptr<BusinessLogger> instance();

    // 初始化业务日志（在程序启动时调用）
    void initialize();

    // 重新配置业务日志（在初始化前调用）
    void configure(const std::string& log_dir,
                   size_t max_file_size = DEFAULT_MAX_FILE_SIZE,
                   size_t max_files = DEFAULT_MAX_FILES);

    // 写入日志 - 通过业务类型
    template<typename... Args>
    void log(BusinessType business_type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(business_type);
        if (logger) {
            logger->log(lvl, fmt, std::forward<Args>(args)...);
        }
    }

    // 便捷方法：各业务类型的日志写入
    template<typename... Args>
    void screen_record(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        log(BusinessType::ScreenRecord, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void desktop_open(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        log(BusinessType::DesktopOpen, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void keyboard_record(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        log(BusinessType::KeyboardRecord, lvl, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void sound_record(level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        log(BusinessType::SoundRecord, lvl, fmt, std::forward<Args>(args)...);
    }

    // 设置全局日志级别
    void set_level(level::level_enum lvl);

    // 设置特定业务的日志级别
    void set_level(BusinessType business_type, level::level_enum lvl);

    // 刷新所有日志
    void flush();

    // 刷新特定业务的日志
    void flush(BusinessType business_type);

private:
    // 获取或创建指定业务的logger
    std::shared_ptr<logger> get_logger(BusinessType business_type);

    // 创建指定业务的logger
    std::shared_ptr<logger> create_logger(BusinessType business_type);

    std::string log_dir_;
    size_t max_file_size_;
    size_t max_files_;
    
    std::map<BusinessType, std::shared_ptr<logger>> loggers_;
    std::mutex mutex_;
    bool initialized_ = false;
};

// 全局便捷函数

// 初始化业务日志系统
inline void init_business_logger(const std::string& log_dir = "logs",
                                  size_t max_file_size = BusinessLogger::DEFAULT_MAX_FILE_SIZE,
                                  size_t max_files = BusinessLogger::DEFAULT_MAX_FILES) {
    auto bl = BusinessLogger::instance();
    bl->configure(log_dir, max_file_size, max_files);
    bl->initialize();
}

// 业务日志宏定义，方便使用
#define BUSINESS_LOG_SCREEN_RECORD(level, ...) \
    spdlog::BusinessLogger::instance()->screen_record(level, __VA_ARGS__)

#define BUSINESS_LOG_DESKTOP_OPEN(level, ...) \
    spdlog::BusinessLogger::instance()->desktop_open(level, __VA_ARGS__)

#define BUSINESS_LOG_KEYBOARD_RECORD(level, ...) \
    spdlog::BusinessLogger::instance()->keyboard_record(level, __VA_ARGS__)

#define BUSINESS_LOG_SOUND_RECORD(level, ...) \
    spdlog::BusinessLogger::instance()->sound_record(level, __VA_ARGS__)

} // namespace spdlog
