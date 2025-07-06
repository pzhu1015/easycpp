// semaphore.h
#pragma once
#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    explicit Semaphore(unsigned int initial_value = 0)
        : count_(initial_value) {}
    
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return count_ > 0; });
        count_--;
    }
    
    bool try_wait() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (count_ > 0) {
            count_--;
            return true;
        }
        return false;
    }
    
    bool timed_wait(long timeout_ms) {
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms), 
                           [this] { return count_ > 0; });
    }
    
    void post() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            count_++;
        }
        cv_.notify_one();
    }

private:
    unsigned int count_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
