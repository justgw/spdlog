// Copyright(c) 2025 Business Logger Extension
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace business_logger {

// 业务类型枚举
enum class BusinessType {
    SCREEN_RECORD,    // 录制屏幕
    DESKTOP_OPEN,     // 打开桌面
    KEYBOARD_RECORD,  // 录制键盘
    AUDIO_RECORD      // 录制声音
};

// BusinessType的哈希函数，用于unordered_map
struct BusinessTypeHash {
    std::size_t operator()(const BusinessType& type) const noexcept {
        return static_cast<std::size_t>(type);
    }
};

// 业务日志管理器类
class BusinessLogManager {
public:
    // 单例模式
    static BusinessLogManager& getInstance();
    
    // 禁用拷贝构造和赋值
    BusinessLogManager(const BusinessLogManager&) = delete;
    BusinessLogManager& operator=(const BusinessLogManager&) = delete;
    
    // 初始化业务日志管理器
    // log_dir: 日志文件存储目录
    // max_file_size: 单个日志文件最大大小（字节），默认30MB
    // max_files: 每个业务保留的最大日志文件数量，默认3个
    void initialize(const std::string& log_dir, 
                   size_t max_file_size = 30 * 1024 * 1024, 
                   size_t max_files = 3);
    
    // 获取指定业务类型的日志记录器
    std::shared_ptr<spdlog::logger> getLogger(BusinessType type);
    
    // 根据业务类型记录日志
    template<typename... Args>
    void log(BusinessType type, spdlog::level::level_enum level, const std::string& fmt, Args&&... args);
    
    // 便捷的日志记录方法
    template<typename... Args>
    void trace(BusinessType type, const std::string& fmt, Args&&... args);
    
    template<typename... Args>
    void debug(BusinessType type, const std::string& fmt, Args&&... args);
    
    template<typename... Args>
    void info(BusinessType type, const std::string& fmt, Args&&... args);
    
    template<typename... Args>
    void warn(BusinessType type, const std::string& fmt, Args&&... args);
    
    template<typename... Args>
    void error(BusinessType type, const std::string& fmt, Args&&... args);
    
    template<typename... Args>
    void critical(BusinessType type, const std::string& fmt, Args&&... args);
    
    // 刷新所有业务日志
    void flushAll();
    
    // 获取业务类型的字符串表示
    static std::string businessTypeToString(BusinessType type);

private:
    BusinessLogManager() = default;
    
    // 创建或获取指定业务类型的日志记录器
    std::shared_ptr<spdlog::logger> createOrGetLogger(BusinessType type);
    
    // 确保日志目录存在
    void ensureLogDirectoryExists(const std::string& log_dir);
    
    std::string log_dir_;                                    // 日志目录
    size_t max_file_size_;                                    // 单个文件最大大小
    size_t max_files_;                                        // 最大文件数量
    std::unordered_map<BusinessType, std::shared_ptr<spdlog::logger>, BusinessTypeHash> loggers_;  // 业务类型到日志记录器的映射
    std::mutex mutex_;                                       // 线程安全锁
    bool initialized_ = false;                                // 初始化标志
};

// 模板方法实现
template<typename... Args>
void BusinessLogManager::log(BusinessType type, spdlog::level::level_enum level, const std::string& fmt, Args&&... args) {
    if (!initialized_) {
        throw std::runtime_error("BusinessLogManager not initialized. Call initialize() first.");
    }
    
    auto logger = getLogger(type);
    if (logger) {
        logger->log(level, fmt, std::forward<Args>(args)...);
    }
}

template<typename... Args>
void BusinessLogManager::trace(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::trace, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogManager::debug(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::debug, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogManager::info(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::info, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogManager::warn(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::warn, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogManager::error(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::err, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void BusinessLogManager::critical(BusinessType type, const std::string& fmt, Args&&... args) {
    log(type, spdlog::level::critical, fmt, std::forward<Args>(args)...);
}

} // namespace business_logger