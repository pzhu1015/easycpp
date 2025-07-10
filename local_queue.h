#pragma once
#include <deque>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <future>
#include <functional>
#include <condition_variable>

namespace queue
{
template<class T>
class LocalQueue
{
public:
    static const int MaxBatchSize = 500;
    using OnConsume = std::function<bool(const std::shared_ptr<T> &data)>;
    using OnBatchConsume = std::function<bool(const std::vector<std::shared_ptr<T>> &data)>;
    LocalQueue()
    {
        this->_started.store(false);
        this->_batch_started.store(false);
    }

    bool Stop()
    {
        this->_started.store(false);
        this->_batch_started.store(false);
        if (this->_task.valid()) 
        {
            this->_task.wait();
        }
        if (this->_batch_task.valid()) 
        {
            this->_batch_task.wait();
        }
        return true;
    }

    bool Publish(const std::shared_ptr<T> &data)
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_datas.emplace_back(data);
        this->_condition.notify_one();
        return true;
    }

    bool Consume(const OnConsume &on_consume)
    {
        if (this->_started.exchange(true)) return true;
        this->_on_consume = on_consume;
        this->_task = std::async(std::launch::async, [this]()
        {
            while(this->_started.load())
            {
                std::shared_ptr<T> data = nullptr;
                {
                    std::unique_lock<std::mutex> lock(this->_mutex);
                    if (this->_datas.empty())
                    {
                        this->_condition.wait_for(lock, std::chrono::seconds(1));
                        continue;
                    }
                    data = this->_datas.front();
                    this->_datas.pop_front();
                }
                if (data && this->_on_consume)
                {
                    this->_on_consume(data);
                }
            }
        });
        return true;
    }

    bool BatchConsume(const OnBatchConsume &on_batch_consume)
    {
        if (this->_batch_started.exchange(true)) return true;
        this->_on_batch_consume = on_batch_consume;
        this->_batch_task = std::async(std::launch::async, [this]()
        {
            while(this->_batch_started.load())
            {
                std::vector<std::shared_ptr<T>> datas;
                {
                    std::unique_lock<std::mutex> lock(this->_mutex);
                    if (this->_datas.empty())
                    {
                        this->_condition.wait_for(lock, std::chrono::seconds(1));
                        continue;
                    }
                    while(!this->_datas.empty() && datas.size() < MaxBatchSize)
                    {
                        datas.emplace_back(this->_datas.front());
                        this->_datas.pop_front();
                    }
                }
                if (!datas.empty() && this->_on_batch_consume)
                {
                    this->_on_batch_consume(datas);
                }
            }
        });
        return true;
    }

private:
    OnConsume                       _on_consume;
    OnBatchConsume                  _on_batch_consume;
    std::atomic<bool>               _started;
    std::atomic<bool>               _batch_started;
    std::future<void>               _task;
    std::future<void>               _batch_task;
    std::mutex                      _mutex;
    std::condition_variable         _condition;
    std::deque<std::shared_ptr<T>>  _datas;
};
}
