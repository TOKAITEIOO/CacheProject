//
// Created by admin on 2025/3/21.
//

#ifndef CACHEPROJECT_KLRUCACHE_H
#define CACHEPROJECT_KLRUCACHE_H

#include <string>
#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "KICachePolicy.h"

template<typename Key, typename Value> class KLruCache;

template<typename Key, typename Value>
class LruNode
{
private:
    Key key_;
    Value value_;
    size_t accessCount_;
    std::shared_ptr<LruNode<Key, Value>> prev_;
    std::shared_ptr<LruNode<Key, Value>> next_;
public:
    LruNode(Key key, Value value)
        :key_(key)
        ,value_(value)
        ,accessCount_(0)
        ,prev_(nullptr)
        ,next_(nullptr)
    {}

    // 提供必要的访问器
    Key getKey() const {return key_;}
    Value getValue() const {return value_;}
    void setValue(const Value& value) {value_ = value;}
    size_t getAccessCount() const {return accessCount_;}
    void incrementAccessCount() {++accessCount_;}

    friend class KLruCache<Key, Value>;
};

template<typename Key, typename Value>
class KLruCache : public KICachePolicy<Key, Value> {
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeMap = std::unordered_map<Key, NodePtr>;

    KLruCache(int capacity)
        :capacity_(capacity)
    {
        initializeList();
    }
    ~KLruCache() override = default;

    void put(Key key, Value value) override {
        if (capacity_ <= 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()) {
            // 如果在当前容器中可以找到，则更新value，并调用get方法，代表该数据刚被访问
            updateExistNode(it->second, value);
            return;
        }
        addNewNode(key, value);
    }
    bool get(Key key, Value& value) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()) {
            // 说明缓存记录中有
            moveToRecent(it->second);
            value = it->second->getValue();

            return true;
        }
        return false;
    }
    Value get(Key key) override {
        Value value{}; // 这里为什么需要加一个{}？
        get(key, value);
        return value;
    }

    // 子类的方法
    void remove(Key key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = nodeMap_.find(key);
        if (it != nodeMap_.end()) {
            removeNode(it->second);
            nodeMap_.erase(it);
        }
    }
private:
    void initializeList() {
        dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
        dummyHead_->next_ = dummyTail_;
        dummyTail_->prev_ = dummyHead_;
    }

    void updateExistNode(NodePtr node, int value) {
        node->setValue(value);
        moveToRecent(node);
    }

    void addNewNode(Key key, Value value) {
        // 首先判断缓存系统是否还有剩余容量
        if(nodeMap_.size() >= capacity_) {
            evictLeastRecent();
        }
        NodePtr node = std::make_shared<LruNodeType>(key, value);
    }

    void moveToRecent(NodePtr node) {
        removeNode(node);
        insertNode(node);
    }

    void removeNode(NodePtr node) {
        node->prev_->next_ = node->next_;
        node->next_->prev_ = node->prev_;
    }

    void insertNode(NodePtr node) {
        node->next_ = dummyTail_->next_;
        node->prev_ = dummyTail_->prev_;
        dummyTail_->prev_->next_ = node;
        dummyTail_->prev_ = node;
    }
    void evictLeastRecent() {
        //淘汰最近最少使用的缓存资源
        NodePtr node = dummyHead_->next_;
        removeNode(node);
        nodeMap_.erase(node->getKey());//还需要在Map中删掉对应的缓存记录
    }
private:
    int capacity_; //判断当前的缓存容量
    NodeMap nodeMap_; // 设置缓存保存的容器
    std::mutex mutex_; // 增加
    NodePtr dummyHead_; //哨兵头节点
    NodePtr dummyTail_; //哨兵尾结点
};

// LRU优化：Lru-k版本。通过继承的方式进行再优化
template<typename Key, typename Value>
class KLruKCache : public KLruCache<Key, Value> {
public:
    KLruKCache(int capacity, int historyCapacity, int k)
        : KLruCache<Key, Value>(capacity) //调用基类构造函数
        , historyList_(std::make_unique<KLruCache<Key, size_t>>(historyCapacity))
        , k_(k)
    {}

    Value get(Key key) {
        // 1. 获取数据的访问次数
        int historyCount = historyList_->get(key);
        // 2. 如果访问到数据，则更新历史访问记录节点值Count++;
        historyList_->put(key, ++historyCount);
        // 从缓存中获取到数据，不一定能获取到，因为可能不在缓存中
        return KLruCache<Key, Value>::get(key);
//        if (historyCount >= k_) {
//            // 移除历史访问记录
//            historyList_->remove(key);
//            KLruCache<Key,Value>::put(key, value);
//        }
    }

    void put(Key key, Value value) {
        // 判断是否在缓存中，如果在则直接替换，不在则不直接添加到缓存中
        if (KLruCache<Key, Value>::get(key, value) == true) {
            KLruCache<Key, Value>::put(key, value);
            return;
        }
        // 增加数据的访问次数，如果数据的访问次数达到上限，则添加进入缓存中
        int historyCount = historyList_->get(key);
        historyList_->put(key, ++historyCount);
        if (historyCount >= k_) {
            // 移除历史访问记录
            historyList_->remove(key);
            KLruCache<Key, Value>::put(key, value);
        }
    }

private:
    int k_; // 进入缓存队列的判断标准
    std::unique_ptr<KLruCache<Key, size_t>> historyList_;
};

// LRU优化：对LRU进行分片，提高并发使用的性能
template<typename Key, typename Value>
class HashLruCache : KLruCache<Key, Value> {
public:
    HashLruCache(size_t capacity, int sliceNum)
        : capacity_(capacity)
        , sliceNum_(sliceNum)
    {}

    void put(Key key, Value value) {
        // 获取key 值的hash值，并计算出对应的索引分片
        size_t sliceIndex = Hash(key) % sliceNum_;
        // 调用对应缓存中的普通lru的put方法
        lruSliceCaches_[sliceIndex]->put(key, value);
    }

    bool get(Key key, Value& value) {
        // 获取key的hash值，并计算出对应的分片索引
        size_t sliceIndex = Hash(key) % sliceNum_;
        lruSliceCaches_[sliceIndex]->get(key, value);
    }

    Value get(Key key) {
        Value value;
        // memset(&value, 0, sizeof(value))
        get(key, value);
        return value;
    }
private:
    // 将对应的key值转换成对应的hash值
    size_t Hash(Key key) {
        std::hash<Key> HashFunc; // 创建一个Hash函数对象
        return HashFunc(key);
    }
private:
    size_t capacity_; // 总容量
    int sliceNum_; // 切片数量
    std::vector<std::unique_ptr<KLruCache<Key, Value>>> lruSliceCaches_; // 切片Lru缓存

};









#endif //CACHEPROJECT_KLRUCACHE_H
