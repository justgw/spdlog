// Copyright(c) 2025 Business Logger Extension
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/business_logger.h"
#include "spdlog/details/os.h"
#include <spdlog/common.h>
#include <stdexcept>
#include <fstream>

namespace business_logger {

BusinessLogManager& BusinessLogManager::getInstance() {
    static BusinessLogManager instance;
    return instance;
}

void BusinessLogManager::initialize(const std::string& log_dir, size_t max_file_size, size_t max_files) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return; // 已经初始化过
    }
    
    log_dir_ = log_dir;
    max_file_size_ = max_file_size;
    max_files_ = max_files;
    
    // 确保日志目录存在
    ensureLogDirectoryExists(log_dir_);
    
    initialized_ = true;
}

std::shared_ptr<spdlog::logger> BusinessLogManager::getLogger(BusinessType type) {
    if (!initialized_) {
        throw std::runtime_error("BusinessLogManager not initialized. Call initialize() first.");
    }
    
    return createOrGetLogger(type);
}

std::shared_ptr<spdlog::logger> BusinessLogManager::createOrGetLogger(BusinessType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查是否已经存在该业务类型的日志记录器
    auto it = loggers_.find(type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    // 创建新的日志记录器
    std::string business_name = businessTypeToString(type);
    std::string logger_name = "business_" + business_name;
    std::string file_path = log_dir_ + "/" + business_name + ".log";
    
    // 创建轮转文件sink，每个文件最大30MB，最多保留3个文件
    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        file_path, max_file_size_, max_files_);
    
    // 创建日志记录器
    auto logger = std::make_shared<spdlog::logger>(logger_name, sink);
    
    // 设置日志格式
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [" + business_name + "] %v");
    
    // 设置日志级别
    logger->set_level(spdlog::level::trace);
    
    // 注册到spdlog
    spdlog::register_logger(logger);
    
    // 保存到映射中
    loggers_[type] = logger;
    
    return logger;
}

void BusinessLogManager::ensureLogDirectoryExists(const std::string& log_dir) {
    if (!spdlog::details::os::path_exists(log_dir)) {
        spdlog::details::os::create_dir(log_dir);
    }
}

void BusinessLogManager::flushAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : loggers_) {
        if (pair.second) {
            pair.second->flush();
        }
    }
}

std::string BusinessLogManager::businessTypeToString(BusinessType type) {
    switch (type) {
        case BusinessType::SCREEN_RECORD:
            return "screen_record";
        case BusinessType::DESKTOP_OPEN:
            return "desktop_open";
        case BusinessType::KEYBOARD_RECORD:
            return "keyboard_record";
        case BusinessType::AUDIO_RECORD:
            return "audio_record";
        default:
            return "unknown";
    }
}

} // namespace business_logger