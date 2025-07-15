#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <thread>
#include <queue>
#include <atomic>
#include <unordered_set>

namespace cache
{

using namespace std::chrono;

template<class K, class V>
class ExpireCache
{
public:
    using OnRange = std::function<bool(const K &key, const std::shared_ptr<V> &value)>;
    using Caches = std::unordered_map<K, std::pair<std::shared_ptr<V>, std::chrono::time_point<std::chrono::steady_clock>>>;
    using OnDelete = std::function<void(const K &key, const std::shared_ptr<V> &value, bool is_manual)>;
    ExpireCache(const OnDelete &on_delete = nullptr, milliseconds timeout = milliseconds::zero()) 
    :
    _started(false),
    _timeout(timeout),
    _on_delete(on_delete)
    {
        start();
    }

    ~ExpireCache()
    {
        stop();
    }

    size_t Count()
    {
        std::shared_lock<std::shared_mutex> read_lock(this->_mutex);
        return this->_cache.size();
    }

    bool Exists(const K &key)
    {
        std::shared_lock<std::shared_mutex> read_lock(this->_mutex);
        auto itr = this->_cache.find(key);
        if (itr == this->_cache.end())
        {
            return false;
        }
        return true;
    }

    std::shared_ptr<V> Get(const K &key)
    {
        std::shared_lock<std::shared_mutex> read_lock(this->_mutex);
        auto itr = this->_cache.find(key);
        if (itr == this->_cache.end())
        {
            return nullptr;
        }
        return itr->second.first;
    }

    std::shared_ptr<V> Delete(const K &key)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        auto itr = this->_cache.find(key);
        if (itr != this->_cache.end())
        {
            auto value = itr->second.first;
            this->_cache.erase(itr);
            write_lock.unlock();
            if (this->_on_delete)
            {
                this->_on_delete(key, value, true);
            }
            return value;
        }
        return nullptr;
    }

    std::pair<std::shared_ptr<V>, bool> Put(const K &key, const std::shared_ptr<V> &value)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        auto expiration = std::chrono::steady_clock::now() + this->_timeout;
        auto itr = this->_cache.insert({key, {value, expiration}});
        if (itr.second && this->_timeout > milliseconds::zero())
        {
            this->_expiration_queue.push({expiration, key});
        }
        return {itr.first->second.first, !itr.second};
    }

    void Range(const OnRange &on_range) 
    {
        Caches snapshot;
        {
            std::shared_lock<std::shared_mutex> read_lock(this->_mutex);
            snapshot = this->_cache;
        }

        for (auto itr : snapshot)
        {
            if (!on_range(itr.first, itr.second.first))
            {
                break;
            }
        }
    }

    void Clear()
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        if (this->_on_delete)
        {
            for (auto& item : this->_cache)
            {
                this->_on_delete(item.first, item.second.first, true);
            }
        }
        this->_cache.clear();
    }

private:
    struct ExpirationEntry
    {
        std::chrono::time_point<std::chrono::steady_clock> expiration;
        K key;

        bool operator>(const ExpirationEntry& other) const
        {
            return expiration > other.expiration;
        }
    };

    void start()
    {
        if (this->_started.exchange(true)) return;
        if (this->_timeout > milliseconds::zero())
        {
            this->_cleanup_thread = std::thread(&ExpireCache<K, V>::cleanup, this);
        }
    }

    void stop()
    {
        this->_started = true;
        if (this->_cleanup_thread.joinable())
        {
            this->_cleanup_thread.join();
        }
    }

    void cleanup()
    {
        std::vector<std::pair<K, std::shared_ptr<V>>> callbacks;
        while (this->_started)
        {
            {
                std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
                while (!this->_expiration_queue.empty() && this->_expiration_queue.top().expiration <= std::chrono::steady_clock::now())
                {
                    auto top = this->_expiration_queue.top();
                    this->_expiration_queue.pop();
                    auto itr = this->_cache.find(top.key);
                    if (itr != this->_cache.end() && itr->second.second == top.expiration)
                    {
                        if (this->_on_delete)
                        {
                            callbacks.emplace_back(itr->first, itr->second.first);
                        }
                        this->_cache.erase(itr);
                    }
                }
            }
            for (auto &cb: callbacks)
            {
                this->_on_delete(cb.first, cb.second, false);
            }
            callbacks.clear();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::shared_mutex           _mutex;
    Caches                      _cache;
    std::thread                 _cleanup_thread;
    std::atomic<bool>           _started;
    std::chrono::milliseconds   _timeout;
    OnDelete                    _on_delete;
    std::priority_queue<ExpirationEntry, std::vector<ExpirationEntry>, std::greater<ExpirationEntry>> _expiration_queue;
};
}
