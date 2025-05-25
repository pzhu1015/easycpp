#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

namespace cache
{
template<class K, class V>
using OnRange = std::function<bool(const K &key, const std::shared_ptr<V> &value)>;

template<class K, class V>
using Caches = std::unordered_map<K, std::shared_ptr<V>>;

template<class K, class V>
class LocalCache
{
public:
    LocalCache() = default;

    ~LocalCache() = default;

    size_t Count()
    {
        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        return this->_cache.size();
    }

    bool Exists(const K &key)
    {
        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        auto itr = this->_cache.find(key);
        if (itr == this->_cache.end())
        {
            return false;
        }
        return true;
    }

    void Put(const K &key, const std::shared_ptr<V>& value)
    {
        std::unique_lock<std::shared_mutex> write_lock(_mutex);
        this->_cache.emplace(key, value);
    }

    std::shared_ptr<V> Get(const K &key)
    {
        std::shared_lock<std::shared_mutex> read_lock(_mutex);
        auto itr = this->_cache.find(key);
        if (itr == this->_cache.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    void Delete(const K &key)
    {
        std::unique_lock<std::shared_mutex> write_lock(_mutex);
        this->_cache.erase(key);
    }

    std::shared_ptr<V> GetAndDelete(const K &key)
    {
        std::unique_lock<std::shared_mutex> write_lock(_mutex);
        auto itr = this->_cache.find(key);
        if (itr != this->_cache.end())
        {
            this->_cache.erase(itr);
            return itr->second;
        }
        return nullptr;
    }

    //添加并获取旧数据, 如果之前不存在返回nullptr
    std::pair<std::shared_ptr<V>, bool> GetAndPut(const K &key, const std::shared_ptr<V> &value)
    {
        std::unique_lock<std::shared_mutex> write_lock(_mutex);
        auto itr = this->_cache.find(key);
        if (itr == this->_cache.end())
        {
            this->_cache.emplace(key, value);
            return {value, false};
        }
        return {itr->second, true};
    }

    void Range(const OnRange<K, V> &fn) 
    {
        Caches<K, V> snapshot;
        {
            std::shared_lock<std::shared_mutex> read_lock(_mutex);
            snapshot = this->_cache;
        }

        for (auto itr : snapshot)
        {
            if (!fn(itr.first, itr.second))
            {
                break;
            }
        }
    }

private:
    std::shared_mutex   _mutex;
    Caches<K,V>         _cache;
};
}
