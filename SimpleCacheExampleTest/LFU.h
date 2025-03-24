//
// Created by admin on 2025/3/24.
//

#ifndef CACHEPROJECT_LFU_H
#define CACHEPROJECT_LFU_H

#include <unordered_map>



class LfuCache {
private:
    // 双向链表的节点
    struct Node {
        int key;
        int value;
        int freq;
        Node *prev, *next;
        Node(int key, int value, int freq) {
            this->key = key;
            this->value = value;
            this->freq = freq;
            this->prev = nullptr;
            this->next = nullptr;
        }
    };
    // 定义双向链表FreqList
    struct FreqList{
        int freq;
        Node *L, *R;
        FreqList(int freq) {
            this->freq = freq;
            L = new Node(-1, -1, 1);
            R = new Node(-1, 1, 1);
            L->next = R;
            R->next = L;
        }
    };
    void deleteFromList(Node *node) {
        Node* prev = node->prev;
        Node* next = node->next;
        prev->next = next;
        next->prev = prev;
    }

    void append(Node* node) {
        /*
        在FreqList中插入对应的Node
         */
        int freq = node->freq;
        if (hashFreq.find(freq) == hashFreq.end()) {
            // 如果对应的freq没有 FreqList，则在hashFreq中创建新的FreqList;
            hashFreq[freq] = new FreqList(freq);
        }
        // 如果有对应的节点，从HashFreq中取出
        FreqList* curList = hashFreq[freq];

        Node* pre = curList->R->prev;
        Node* next = curList->R;
        pre->next = node;
        node->next = next;
        next->prev = node;
        node->prev = pre;

    }

private:
    int n; //缓存空间的大小
    /*
    整个缓存中的节点最小访问频数， LFU 中为每一个频数构造一个双向链表，
    当缓存空间满了时，首先需要知道当前缓存中最小的频数是多少，再需要找到
    该最小频数下最久未使用的数据淘汰。想要在 O(1) 时间复杂度下完成上述
    操作，处理使用双向链表结构，还需要动态记录维护缓存空间中最小访问频数 minFreq
     */
    int minFreq;
    std::unordered_map<int, Node*> hashNode; // 节点hash表，用于快速获取数据Key中对应的节点信息
    std::unordered_map<int, FreqList*> hashFreq; //频数双向链表哈希表，为每个访问频数构造一个双向链表，并且用哈希表联系二者关系

public:
    // LFUCache的构造函数
    LfuCache(int capacity)
        :n(capacity)
        ,minFreq(0)
    {}

    // 访问缓存数据
    int get(int key) {
        if (hashNode.find(key) != hashNode.end()) {
            Node *node = hashNode[key]; // 利用节点哈希表，O(1)时间复杂度下定位到该节点
            // 每次get操作会将该节点的访问频数+1,所以需要将它从原来的频数对应的双向链表中删除
            deleteFromList(node);
            node->freq++;
            /*
             * 下面这个操作是
             *
             * */

            return hashNode[key]->value;
        }

    }

};







#endif //CACHEPROJECT_LFU_H
