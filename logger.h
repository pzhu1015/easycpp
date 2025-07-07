#pragma once
#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace logger
{
// 日志级别
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// 日志配置
struct LogConfig {
    static LogLevel min_level;  // 最小日志级别
    static std::mutex mutex;
};

// 初始化配置默认值
LogLevel LogConfig::min_level = LogLevel::DEBUG;
std::mutex LogConfig::mutex;

// ANSI 颜色代码
constexpr const char* COLOR_DEBUG = "\033[36m";   // 青色
constexpr const char* COLOR_INFO = "\033[32m";    // 绿色
constexpr const char* COLOR_WARNING = "\033[33m"; // 黄色
constexpr const char* COLOR_ERROR = "\033[31m";   // 红色
constexpr const char* COLOR_RESET = "\033[0m";    // 重置颜色

// 获取当前时间字符串
static std::string current_time() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    
    std::tm bt = *std::localtime(&timer);
    std::ostringstream oss;
    
    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

// 核心日志函数
static void log_message(LogLevel level, const char* file, int line, const char* fmt, ...) {
    std::unique_lock<std::mutex> lock(LogConfig::mutex);
    // 级别过滤
    if (level < LogConfig::min_level) return;
    
    // 获取可变参数
    va_list args;
    va_start(args, fmt);
    
    const char* level_str = "";
    const char* color = "";
    switch(level) 
    {
    case LogLevel::DEBUG:   level_str = "DEBUG";   color = COLOR_DEBUG;   break;
    case LogLevel::INFO:    level_str = "INFO";    color = COLOR_INFO;    break;
    case LogLevel::WARNING: level_str = "WARNING"; color = COLOR_WARNING; break;
    case LogLevel::ERROR:   level_str = "ERROR";   color = COLOR_ERROR;   break;
    }
        
    // 打印前缀
    std::fprintf(stdout, "[%s][%s:%d]%s[%s]%s ", current_time().data(), file, line, color, level_str, COLOR_RESET);
    
    // 打印用户消息
    std::vfprintf(stdout, fmt, args);
    
    // 添加换行符
    std::fprintf(stdout, "\n");
    
    va_end(args);
}
//获取文件名
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

// 日志宏 (自动捕获文件和行号)
#define DEBUG(fmt, ...)   ::logger::log_message(::logger::LogLevel::DEBUG,   __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)    ::logger::log_message(::logger::LogLevel::INFO,    __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define WARNING(fmt, ...) ::logger::log_message(::logger::LogLevel::WARNING, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)   ::logger::log_message(::logger::LogLevel::ERROR,   __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
}

