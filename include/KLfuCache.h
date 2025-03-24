//
// Created by admin on 2025/3/24.
//

#ifndef CACHEPROJECT_KLFUCACHE_H
#define CACHEPROJECT_KLFUCACHE_H

#include "KICachePolicy.h"

#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <string>
#include <list>

template<typename Key, typename Value> class KfuCache;

template<typename Key, typename Value>
class FreqList{
private:
    // 创建节点的基础形式
    struct Node{
        Key key;
        Value value;
        int freq;
        std::shared_ptr<Node> pre; //上一个节点
        std::shared_ptr<Node> next; // 下一个节点

        Node()
            :freq(1)
            ,pre(nullptr)
            ,next(nullptr)
        {}
        Node(Key key, Value value)
            :freq(1)
            ,key(key)
            ,value(value)
            ,pre(nullptr)
            ,next(nullptr)
        {}
    };
    using NodePtr = std::shared_ptr<Node>;
    int freq_; //访问频率
    NodePtr dummyHead_; //定义虚拟头结点
    NodePtr dummytail_; //定义虚拟尾节点
public:
    explicit FreqList(int n)
        : freq_(n)
    {
        dummyHead_ = std::make_shared<Node>(); //创建头节点
        dummytail_ = std::make_shared<Node>();
        dummyHead_->next = dummytail_; // 链接虚拟头结点和尾节点
        dummytail_->pre = dummyHead_;
    }
    bool isEmpty() const {
        return dummyHead_->next == dummytail_;
    }

    //添加节点的管理方法
    void addNode(NodePtr node) {
        if (!node || !dummyHead_ || !dummytail_) {
            // 如果为空节点则直接返回
            return;
        }
        node->pre = dummytail_->pre;
        node->next = dummytail_->next;
        dummytail_->pre->next = node;
        dummytail_->pre = node;
    }

    // 移除节点的管理方法
    void removeNode(NodePtr node) {
        if (!node || !dummyHead_ || !dummyHead_) {
            return;
        }
        if (!node->pre || !node->next) {
            return;
        }
        node->pre->next = node->next;
        node->next->pre = node->pre;
        // 这里不delete Node是否会造成内存泄漏？
        node->pre = nullptr;
        node->next = nullptr;
    }
    // 返回第一个Node
    NodePtr getFirstNode() const {return dummyHead_->next;}

    friend class KfuCache<Key, Value>;
};







#endif //CACHEPROJECT_KLFUCACHE_H
