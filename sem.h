#pragma once
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <ctime>
class Semaphore {
public:
    // 构造函数，初始化信号量
    explicit Semaphore(unsigned int initial_value = 0) {
        if (sem_init(&sem_, 0, initial_value) != 0) {
            throw std::runtime_error(std::strerror(errno));
        }
    }
    // 析构函数，销毁信号量
    ~Semaphore() {
        sem_destroy(&sem_);
    }
    // 等待信号量（P操作）
    void wait() {
        while (true) {
            if (sem_wait(&sem_) == 0) {
                return;
            }
            // 如果被信号中断，则继续等待
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::strerror(errno));
        }
    }
    // 尝试等待信号量（非阻塞）
    bool try_wait() {
        int ret = sem_trywait(&sem_);
        if (ret == 0) {
            return true;
        }
        if (errno == EAGAIN) {
            return false;
        }
        throw std::runtime_error(std::strerror(errno));
    }
    // 带超时的等待信号量
    bool timed_wait(long millis) {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
            throw std::runtime_error(std::strerror(errno));
        }
        // 计算绝对超时时间
        ts.tv_sec += millis / 1000;
        ts.tv_nsec += (millis % 1000) * 1000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }
        int ret;
        while ((ret = sem_timedwait(&sem_, &ts)) != 0) {
            if (errno == EINTR) {
                continue; // 被信号中断，继续等待
            }
            if (errno == ETIMEDOUT) {
                return false; // 超时
            }
            throw std::runtime_error(std::strerror(errno));
        }
        return true;
    }
    // 释放信号量（V操作）
    void post() {
        if (sem_post(&sem_) != 0) {
            throw std::runtime_error(std::strerror(errno));
        }
    }
private:
    sem_t sem_;
    // 禁止拷贝
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
};
