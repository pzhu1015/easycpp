#pragma once
#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <cmath>
#include <algorithm>

namespace ratelimit
{
class RateLimiter {
public:
    // 构造函数：每秒生成 rate 个令牌，桶容量为 burst
    RateLimiter(double rate, int64_t burst)
        : rate_(rate), 
          burst_(burst), 
          tokens_(static_cast<double>(burst)),
          last_(std::chrono::steady_clock::now()) {}

    // 尝试获取一个令牌（非阻塞）
    bool Allow() {
        return AllowN(1);
    }

    // 尝试获取n个令牌（非阻塞）
    bool AllowN(int64_t n) {
        std::lock_guard<std::mutex> lock(mutex_);
        refill();
        if (tokens_ < n) return false;
        tokens_ -= n;
        return true;
    }

    // 阻塞等待一个令牌
    void Wait() {
        WaitN(1);
    }

    // 阻塞等待n个令牌
    void WaitN(int64_t n) {
        // 直接等待所需时间
        std::this_thread::sleep_for(ReserveN(n));
    }

    // 预约n个令牌，返回需要等待的时间
    std::chrono::duration<double> ReserveN(int64_t n) {
        std::lock_guard<std::mutex> lock(mutex_);
        refill();
        
        // 1. 如果有足够令牌，立即返回
        if (tokens_ >= n) {
            tokens_ -= n;
            return std::chrono::duration<double>(0);
        }
        
        // 2. 计算需要等待的时间（考虑令牌债务）
        double needed = n - tokens_;
        double wait_seconds = needed / rate_;
        
        // 3. 扣除令牌（允许负值表示债务）
        tokens_ -= n;
        
        return std::chrono::duration<double>(wait_seconds);
    }

private:
    // 补充令牌（不修改last_时间戳）
    void refill() {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_).count();
        
        // 只处理正时间流逝
        if (elapsed <= 0) return;
        
        // 计算新增令牌（不超过桶容量）
        double new_tokens = elapsed * rate_;
        tokens_ = std::min(tokens_ + new_tokens, static_cast<double>(burst_));
        last_ = now;
    }

    const double rate_;          // 令牌生成速率（个/秒）
    const int64_t burst_;        // 桶容量
    double tokens_;              // 当前令牌数量（可负）
    std::chrono::steady_clock::time_point last_; // 上次补充时间
    std::mutex mutex_;           // 互斥锁
};
}
