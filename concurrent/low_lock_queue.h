#ifndef __LEI_UTILS_LOW_LOCK_QUEUE_H__
#define __LEI_UTILS_LOW_LOCK_QUEUE_H__

#include <atomic>
#include <thread>

namespace lei {
namespace utils {

static const uint16_t kCacheLineSize = 64;    // mainstream cache-line size is 64 Bytes

template <typename T>
class LowLockQueue {
public:    
    LowLockQueue(uint32_t capacity)
        : m_capacity(capacity),
          m_size(0) {
        m_consumer_lock = m_producer_lock = false;
        m_first = m_last = new Node(nullptr);
    }

    ~LowLockQueue() {
        while (m_first != nullptr) {
            Node *old_first = m_first;
            m_first = m_first->next;
            delete old_first->value;
            delete old_first; 
        }
        m_last = nullptr;
    }

    LowLockQueue(LowLockQueue &&) = delete;
    LowLockQueue(const LowLockQueue &) = delete;
    LowLockQueue& operator=(const LowLockQueue &) = delete;

    bool EnQueue(const T &t) {
        if (m_size.load() >= get_capacity()) { return false; }

        Node *node = new Node(new T(t));
        while (m_producer_lock.exchange(true)) {
            std::this_thread::yield();
        }
        m_last->next = node;
        m_last = node;
        m_size.fetch_add(1);
        m_producer_lock = false;
        return true;
    }

    bool DeQueue(T &result) {
        if (empty()) { return false; };

        while (m_consumer_lock.exchange(true)) {
            std::this_thread::yield();
        }
        Node *the_first = m_first;
        Node *the_next = m_first->next;

        if (the_next != nullptr) { 
            T *val = the_next->value;
            the_next->value = nullptr;
            m_first = the_next;
            m_size.fetch_sub(1);
            m_consumer_lock = false;    // release lock

            result = std::move(*val);
            delete val;
            delete the_first; 
            return true;
        }

        m_consumer_lock = false;
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
        char padding[kCacheLineSize - sizeof(T*) - sizeof(std::atomic<Node*>)];
    };

    uint32_t m_capacity;
    char padding0[kCacheLineSize - sizeof(uint32_t)];

    std::atomic<uint32_t> m_size;
    char padding1[kCacheLineSize - sizeof(std::atomic<uint32_t>)];

    Node* m_first;
    char padding2[kCacheLineSize - sizeof(Node*)];

    std::atomic<bool> m_consumer_lock;
    char padding3[kCacheLineSize - sizeof(std::atomic<bool>)];

    Node* m_last;
    char padding4[kCacheLineSize - sizeof(Node*)];

    std::atomic<bool> m_producer_lock;
    char padding5[kCacheLineSize - sizeof(std::atomic<bool>)];
};

}
}

#endif
