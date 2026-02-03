// Copyright(c) 2025 spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include <spdlog/business_logger.h>
#include <spdlog/common.h>

#include <sys/stat.h>
#include <iostream>

namespace spdlog {

// 单例实例
static std::shared_ptr<BusinessLogger> g_business_logger_instance;
static std::mutex g_instance_mutex;

BusinessLogger::BusinessLogger(const std::string& log_dir,
                               size_t max_file_size,
                               size_t max_files)
    : log_dir_(log_dir),
      max_file_size_(max_file_size),
      max_files_(max_files),
      initialized_(false) {
}

std::shared_ptr<BusinessLogger> BusinessLogger::instance() {
    std::lock_guard<std::mutex> lock(g_instance_mutex);
    if (!g_business_logger_instance) {
        g_business_logger_instance = std::shared_ptr<BusinessLogger>(
            new BusinessLogger());
    }
    return g_business_logger_instance;
}

void BusinessLogger::configure(const std::string& log_dir,
                               size_t max_file_size,
                               size_t max_files) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }
    log_dir_ = log_dir;
    max_file_size_ = max_file_size;
    max_files_ = max_files;
}

void BusinessLogger::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (initialized_) {
        return;
    }

    // 创建日志目录
    #ifdef _WIN32
        _mkdir(log_dir_.c_str());
    #else
        mkdir(log_dir_.c_str(), 0755);
    #endif

    // 为每种业务类型创建logger
    create_logger(BusinessType::ScreenRecord);
    create_logger(BusinessType::DesktopOpen);
    create_logger(BusinessType::KeyboardRecord);
    create_logger(BusinessType::SoundRecord);

    initialized_ = true;
}

std::shared_ptr<logger> BusinessLogger::get_logger(BusinessType business_type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loggers_.find(business_type);
    if (it != loggers_.end()) {
        return it->second;
    }
    
    // 如果logger不存在，创建它
    return create_logger(business_type);
}

std::shared_ptr<logger> BusinessLogger::create_logger(BusinessType business_type) {
    std::string business_name = business_type_to_string(business_type);
    std::string log_file = log_dir_ + "/" + business_name + ".log";
    
    try {
        // 创建旋转文件sink
        auto sink = std::make_shared<sinks::rotating_file_sink_mt>(
            log_file, max_file_size_, max_files_, false);
        
        // 创建logger
        auto new_logger = std::make_shared<logger>(business_name, sink);
        new_logger->set_level(level::info);
        
        // 设置日志格式: [时间] [级别] 消息
        new_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        
        // 注册到spdlog全局注册表
        register_logger(new_logger);
        
        // 保存到本地map
        loggers_[business_type] = new_logger;
        
        return new_logger;
    } catch (const spdlog_ex& ex) {
        std::cerr << "Failed to create logger for " << business_name 
                  << ": " << ex.what() << std::endl;
        return nullptr;
    }
}

void BusinessLogger::set_level(level::level_enum lvl) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        if (pair.second) {
            pair.second->set_level(lvl);
        }
    }
}

void BusinessLogger::set_level(BusinessType business_type, level::level_enum lvl) {
    auto logger = get_logger(business_type);
    if (logger) {
        logger->set_level(lvl);
    }
}

void BusinessLogger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : loggers_) {
        if (pair.second) {
            pair.second->flush();
        }
    }
}

void BusinessLogger::flush(BusinessType business_type) {
    auto logger = get_logger(business_type);
    if (logger) {
        logger->flush();
    }
}

} // namespace spdlog
