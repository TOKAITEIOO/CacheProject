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
    struct Node{
        Key key;
        Value value;
        int freq;
        std::shared_ptr<Node> pre; //上一个节点
        std::shared_ptr<Node> next; // 下一个节点

        Node(): freq(1)
    };

};





#endif //CACHEPROJECT_KLFUCACHE_H
