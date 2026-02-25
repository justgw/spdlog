// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/common.h>

#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>

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
        default:                           return "unknown";
    }
}

// 多业务日志管理器配置结构
struct MultiBusinessLoggerConfig {
    std::string log_dir = "logs";           // 日志文件目录
    size_t max_file_size = 30 * 1024 * 1024; // 单个文件最大大小 (30MB)
    size_t max_files = 3;                    // 最多保留文件数
    bool rotate_on_open = false;             // 打开时是否轮转
    level::level_enum level = level::info;   // 默认日志级别
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"; // 日志格式
};

// 多业务日志管理器类
// 支持根据业务类型写入不同的日志文件，每个业务独立轮转
class MultiBusinessLogger {
public:
    // 构造函数
    explicit MultiBusinessLogger(const MultiBusinessLoggerConfig& config = {});
    
    // 析构函数
    ~MultiBusinessLogger() = default;
    
    // 禁用拷贝
    MultiBusinessLogger(const MultiBusinessLogger&) = delete;
    MultiBusinessLogger& operator=(const MultiBusinessLogger&) = delete;
    
    // 允许移动
    MultiBusinessLogger(MultiBusinessLogger&&) noexcept = default;
    MultiBusinessLogger& operator=(MultiBusinessLogger&&) noexcept = default;

    // 写入日志 - 使用业务类型枚举
    template<typename... Args>
    void log(BusinessType business_type, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(business_type);
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }

    // 写入日志 - 使用字符串标识业务类型
    template<typename... Args>
    void log(const std::string& business_key, level::level_enum lvl, format_string_t<Args...> fmt, Args&&... args) {
        auto logger = get_logger(business_key);
        logger->log(lvl, fmt, std::forward<Args>(args)...);
    }

    // 便捷方法：各日志级别
    template<typename... Args>
    void trace(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(BusinessType business_type, format_string_t<Args...> fmt, Args&&... args) {
        log(business_type, level::critical, fmt, std::forward<Args>(args)...);
    }

    // 字符串业务键的便捷方法
    template<typename... Args>
    void trace(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(const std::string& business_key, format_string_t<Args...> fmt, Args&&... args) {
        log(business_key, level::critical, fmt, std::forward<Args>(args)...);
    }

    // 设置特定业务的日志级别
    void set_level(BusinessType business_type, level::level_enum lvl);
    void set_level(const std::string& business_key, level::level_enum lvl);
    
    // 设置全局日志级别
    void set_global_level(level::level_enum lvl);

    // 刷新特定业务的日志
    void flush(BusinessType business_type);
    void flush(const std::string& business_key);
    
    // 刷新所有日志
    void flush_all();

    // 获取特定业务的 logger
    std::shared_ptr<logger> get_logger(BusinessType business_type);
    std::shared_ptr<logger> get_logger(const std::string& business_key);

    // 检查业务 logger 是否存在
    bool has_logger(BusinessType business_type) const;
    bool has_logger(const std::string& business_key) const;

private:
    // 创建 logger 的内部方法
    std::shared_ptr<logger> create_logger(const std::string& business_key);
    std::string get_log_filename(const std::string& business_key) const;

    MultiBusinessLoggerConfig config_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<logger>> loggers_;
};

// 全局多业务日志管理器实例（可选使用）
class MultiBusinessLoggerRegistry {
public:
    static MultiBusinessLoggerRegistry& instance();
    
    // 初始化全局管理器
    void initialize(const MultiBusinessLoggerConfig& config);
    
    // 获取全局管理器
    std::shared_ptr<MultiBusinessLogger> get_logger();
    
    // 检查是否已初始化
    bool is_initialized() const;

private:
    MultiBusinessLoggerRegistry() = default;
    ~MultiBusinessLoggerRegistry() = default;
    
    MultiBusinessLoggerRegistry(const MultiBusinessLoggerRegistry&) = delete;
    MultiBusinessLoggerRegistry& operator=(const MultiBusinessLoggerRegistry&) = delete;

    std::shared_ptr<MultiBusinessLogger> logger_;
    mutable std::mutex mutex_;
};

// 便捷函数：获取全局多业务日志管理器
inline std::shared_ptr<MultiBusinessLogger> get_multi_business_logger();

// 便捷函数：初始化全局多业务日志管理器
inline void init_multi_business_logger(const MultiBusinessLoggerConfig& config = {});

} // namespace spdlog

// 包含内联实现
#include "multi_business_logger-inl.h"
