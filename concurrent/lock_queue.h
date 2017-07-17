#ifndef __LEI_UTILS_LOCK_QUEUE_H__
#define __LEI_UTILS_LOCK_QUEUE_H__

#include <atomic>
#include <thread>
#include <mutex>

namespace lei {
namespace utils {

static const uint16_t kCacheLineSize_ = 64;    // mainstream cache-line size is 64 Bytes

template <typename T>
class LockQueue {
public:    
    LockQueue(uint32_t capacity)
        : m_capacity(capacity),
          m_size(0) {
        m_first = m_last = new Node(nullptr);
    }

    ~LockQueue() {
        while (m_first != nullptr) {
            Node *old_first = m_first;
            m_first = m_first->next;
            delete old_first->value;
            delete old_first; 
        }
        m_last = nullptr;
    }

    LockQueue(LockQueue &&) = delete;
    LockQueue(const LockQueue &) = delete;
    LockQueue& operator=(const LockQueue &) = delete;

    bool EnQueue(const T &t) {
        if (m_size.load() >= get_capacity()) { return false; }

        Node *node = new Node(new T(t));

        std::lock_guard<std::mutex> lock(m_producer_lock);
        m_last->next = node;
        m_last = node;
        m_size.fetch_add(1);
        return true;
    }

    bool DeQueue(T &result) {
        if (empty()) { return false; }
        std::unique_lock<std::mutex> lock(m_consumer_lock);
        Node *the_first = m_first;
        Node *the_next = m_first->next;

        if (the_next != nullptr) { 
            T *val = the_next->value;
            the_next->value = nullptr;
            m_first = the_next;
            m_size.fetch_sub(1);
            lock.unlock();

            result = std::move(*val);
            delete val;
            delete the_first; 
            return true;
        }

        lock.unlock();
        return false;
    }

    uint32_t inline get_capacity() const { return m_capacity; }

    uint32_t size() { return m_size.load(); }

    bool empty() { return m_size.load() == 0; }

private:
    struct Node {
        Node(T *val): value(val), next(nullptr) {}
        T* value;
        std::atomic<Node*> next;
        char padding[kCacheLineSize_ - sizeof(T*) - sizeof(std::atomic<Node*>)];
    };

    uint32_t m_capacity;
    char padding0[kCacheLineSize_ - sizeof(uint32_t)];

    std::atomic<uint32_t> m_size;
    char padding1[kCacheLineSize_ - sizeof(std::atomic<uint32_t>)];

    Node* m_first;
    char padding2[kCacheLineSize_ - sizeof(Node*)];

    std::mutex m_consumer_lock;
    char padding3[kCacheLineSize_ - sizeof(std::mutex)];

    Node* m_last;
    char padding4[kCacheLineSize_ - sizeof(Node*)];

    std::mutex m_producer_lock;
    char padding5[kCacheLineSize_ - sizeof(std::mutex)];

};

}
}

#endif
