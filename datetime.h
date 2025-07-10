#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstring> // memcpy
#ifdef HAVE_MYSQL  // 仅在使用 MySQL 时启用
#include <mysql/mysql.h>
#endif

namespace datetime
{
class DateTime {
public:
    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;
    using Days = std::chrono::duration<int, std::ratio<86400>>;

    // 默认构造函数（零值时间）
    DateTime() : is_zero_(true) {}

#ifdef HAVE_MYSQL  // 仅在使用 MySQL 时启用
    explicit DateTime(const st_mysql_time& t)
    {
        if (t.year == 0 && t.month == 0 && t.day == 0)
        {
            is_zero_ = true;
        }
        else
        {
            std::tm tm = {};
            tm.tm_year = t.year - 1900;
            tm.tm_mon = t.month - 1;
            tm.tm_mday = t.day;
            tm.tm_hour = t.hour;
            tm.tm_min = t.minute;
            tm.tm_sec = t.second;
            tp_ = Clock::from_time_t(std::mktime(&tm));
            is_zero_ = false;
        }
    }
#endif

    // 从time_point构造
    explicit DateTime(const TimePoint& tp) : tp_(tp), is_zero_(false) {}
    
    // 从 time_t 构造
    explicit DateTime(std::time_t t) : tp_(Clock::from_time_t(t)), is_zero_(false) {}

    // 从MySQL datetime字符串构造
    explicit DateTime(const std::string& str, const std::string &format = "%Y-%m-%d %H:%M:%S") 
    {
        *this = DateTime::Parse(str, format);
    }

    // 转换为 time_t
    std::time_t UnixTime() const 
    {
        if (is_zero_) return 0;
        return Clock::to_time_t(tp_);
    }

    // 转换为字符串
    std::string ToString(const std::string &format = "%Y-%m-%d %H:%M:%S") const {
        if (is_zero_) {
            return "";
        }
        
        std::time_t t = Clock::to_time_t(tp_);
        std::tm tm;
        localtime_r(&t, &tm);
        std::ostringstream ss;
        ss << std::put_time(&tm, format.data());
        return ss.str();
    }

    // 检查是否为零值
    bool IsZero() const { return is_zero_; }

    // 比较操作符（零值视为最小时间）
    bool operator<(const DateTime& other) const 
    {
        if (is_zero_ && !other.is_zero_) return true;
        if (!is_zero_ && other.is_zero_) return false;
        if (is_zero_ && other.is_zero_) return false;
        return tp_ < other.tp_;
    }
    
    bool operator>(const DateTime& other) const { return other < *this; }
    bool operator<=(const DateTime& other) const { return !(*this > other); }
    bool operator>=(const DateTime& other) const { return !(*this < other); }
    bool operator!=(const DateTime& other) const { return !(*this == other); }
    bool operator==(const DateTime& other) const 
    {
        if (is_zero_ != other.is_zero_) return false;
        return is_zero_ || tp_ == other.tp_;
    }

#ifdef HAVE_MYSQL  // 仅在使用 MySQL 时启用
    // 转换为 st_mysql_time
    operator st_mysql_time() const
    {
        st_mysql_time t = {};
        if (!is_zero_)
        {
            std::time_t tt = Clock::to_time_t(tp_);
            std::tm tm = *std::localtime(&tt);
            t.year = tm.tm_year + 1900;
            t.month = tm.tm_mon + 1;
            t.day = tm.tm_mday;
            t.hour = tm.tm_hour;
            t.minute = tm.tm_min;
            t.second = tm.tm_sec;
        }
        return t;
    }
#endif
    
    // 时间运算方法（零值保持不变）
    DateTime AddYears(int years) const 
    {
        if (is_zero_) return *this;
        std::time_t t = Clock::to_time_t(tp_);
        std::tm tm;
        localtime_r(&t, &tm);
        tm.tm_year += years;
        return DateTime(Clock::from_time_t(std::mktime(&tm)));
    }
    
    DateTime AddMonths(int months) const 
    {
        if (is_zero_) return *this;
        std::time_t t = Clock::to_time_t(tp_);
        std::tm tm;
        localtime_r(&t, &tm);
        int total_months = tm.tm_mon + months;
        tm.tm_mon = total_months % 12;
        tm.tm_year += total_months / 12;
        // 处理月末对齐
        if (tm.tm_mday > DaysInMonth(tm.tm_year + 1900, tm.tm_mon + 1)) 
        {
            tm.tm_mday = DaysInMonth(tm.tm_year + 1900, tm.tm_mon + 1);
        }
        return DateTime(Clock::from_time_t(std::mktime(&tm)));
    }
    
    DateTime AddDays(int days) const 
    {
        if (is_zero_) return *this;
        return DateTime(tp_ + Days(days));
    }

    // 统一的时间增量方法，接受任何 std::chrono 时间单位
    template <typename Duration>
    DateTime Add(const Duration& duration) const 
    {
        if (is_zero_) return *this;
        return DateTime(tp_ + duration);
    }
    
    DateTime AddHours(int hours) const 
    {
        return Add(std::chrono::hours(hours));
    }
    
    DateTime AddMinutes(int minutes) const 
    {
        return Add(std::chrono::minutes(minutes));
    }
    
    DateTime AddSeconds(int seconds) const 
    {
        return Add(std::chrono::seconds(seconds));
    }

    TimePoint GetTimePoint() const 
    { 
        if (is_zero_) return TimePoint{};
        return tp_; 
    }

    // 计算某年某月的天数
    static int DaysInMonth(int year, int month) 
    {
        static const int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && IsLeapYear(year)) 
        {
            return 29;
        }
        return days[month - 1];
    }

    // 判断闰年
    static bool IsLeapYear(int year) 
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static DateTime Now()
    {
        return DateTime(Clock::now());
    }

    static DateTime Parse(const std::string &str, const std::string &format = "%Y-%m-%d %H:%M:%S")
    {
        if (str.empty() || str.find("0000-00-00") != std::string::npos) 
        {
            return DateTime();
        }
        
        std::tm tm = {};
        std::istringstream ss(str);
        ss >> std::get_time(&tm, format.data());
        if (ss.fail()) 
        {
            return DateTime();
        }
        return DateTime(Clock::from_time_t(std::mktime(&tm)));
    }

private:
    TimePoint tp_;
    bool is_zero_ = false;
};
}
