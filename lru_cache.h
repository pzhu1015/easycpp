#pragma once
#include <list>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <string>
#include <memory>

namespace cache
{
// 通用线程安全 LRU 缓存模板
template <typename Key, 
          typename Value, 
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class LRUCache {
public:
    using Deleter = std::function<void(const Key &key, Value value)>;

    // 构造函数
    explicit LRUCache(
        size_t capacity,
        Deleter deleter = nullptr
    ) : capacity_(capacity), deleter_(deleter) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be greater than 0");
        }
    }

    // 析构函数 - 自动清理所有资源
    ~LRUCache() {
        clear();
    }

    // 添加或更新元素
    void put(const Key& key, Value value) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // 更新现有元素
            it->second->value = value;
            // 移动到最近使用位置
            move_to_front(it->second);
            return;
        }

        // 检查容量并淘汰最旧项
        if (cache_list_.size() >= capacity_) {
            evict_last();
        }

        // 添加新项到列表头部
        auto new_item = std::make_shared<CacheItem>(key, value);
        cache_list_.push_front(new_item);
        cache_map_[key] = new_item;
    }

    // 获取元素（可选是否标记为使用）
    Value get(const Key& key, bool mark_used = true) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return nullptr; // 或根据 Value 类型返回默认值
        }
        
        if (mark_used) {
            move_to_front(it->second);
        }
        
        return it->second->value;
    }

    // 检查是否存在元素
    bool contains(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_map_.find(key) != cache_map_.end();
    }

    // 显式移除元素
    bool remove(const Key& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return false;
        }

        auto item = it->second;
        // 从映射和列表中移除
        cache_map_.erase(it);
        cache_list_.remove(item);
        
        // 销毁资源
        if (deleter_) {
            deleter_(item->key, item->value);
        }
        
        return true;
    }

    // 清空整个缓存
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (deleter_) {
            for (auto& item : cache_list_) {
                deleter_(item->key, item->value);
            }
        }
        
        cache_list_.clear();
        cache_map_.clear();
    }

    // 获取当前大小
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_list_.size();
    }

    // 获取容量
    size_t capacity() const {
        return capacity_;
    }

private:
    struct CacheItem {
        Key key;
        Value value;
        
        CacheItem(const Key& k, Value v) : key(k), value(v) {}
    };

    using ItemPtr = std::shared_ptr<CacheItem>;
    using ItemList = std::list<ItemPtr>;
    
    void evict_last() {
        if (cache_list_.empty()) return;
        
        auto last = cache_list_.back();
        cache_map_.erase(last->key);
        cache_list_.pop_back();
        
        if (deleter_) {
            deleter_(last->key, last->value);
        }
    }

    void move_to_front(const ItemPtr& item) {
        // 从当前位置移除
        cache_list_.remove(item);
        // 添加到头部
        cache_list_.push_front(item);
    }

    size_t capacity_;
    ItemList cache_list_; // 最近使用的在列表前端
    std::unordered_map<Key, ItemPtr, Hash, KeyEqual> cache_map_;
    Deleter deleter_;
    mutable std::mutex mutex_; // 保证线程安全
};
}

