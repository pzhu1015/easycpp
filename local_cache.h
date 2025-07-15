#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

namespace cache
{

template<class K, class V>
class LocalCache
{
public:
    using OnRange = std::function<bool(const K &key, const std::shared_ptr<V> &value)>;
    using Caches = std::unordered_map<K, std::shared_ptr<V>>;
    LocalCache() = default;
    ~LocalCache() = default;

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
        return itr->second;
    }

    std::shared_ptr<V> Delete(const K &key)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        auto itr = this->_cache.find(key);
        if (itr != this->_cache.end())
        {
            this->_cache.erase(itr);
            return itr->second;
        }
        return nullptr;
    }

    std::pair<std::shared_ptr<V>, bool> Put(const K &key, const std::shared_ptr<V> &value)
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        auto itr = this->_cache.insert({key, value});
        return {itr.first->second, !itr.second};
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
            if (!on_range(itr.first, itr.second))
            {
                break;
            }
        }
    }

    void Clear()
    {
        std::unique_lock<std::shared_mutex> write_lock(this->_mutex);
        this->_cache.clear();
    }

private:
    std::shared_mutex   _mutex;
    Caches              _cache;
};
}
