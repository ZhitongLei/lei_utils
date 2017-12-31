#ifndef __LEI_UTILS_H__
#define __LEI_UTILS_H__

// A lock-free stack (implemented through linked list) 
// 使用shared_ptr的计数解决ABA问题（内存回收问题）
// 想象这种场景：
// 当前链表的元素为 A-->B-->C, head=A
// 线程1在进行pop操作，线程2也在进行pop操作，将A、B都出栈了，将A、B内存释放，并重新将new A入栈，此时链表变成A-->C
// (1) 如果使用裸指针，操作系统很有可能会重用A的内存，但是从逻辑上说，已经不是换了一个节点了，
// 此时CAS会认为head的值没变，还是原来的A，因此会将head设为A-->next，也就是已经出栈的B，如果B的内存已经释放，操作结果是未定义的
// (2) 改用 shared_ptr的话，由于新节点的next指针仍然指向旧A，A的内存不会释放，也就不存在内存重用问题，不会出现新入栈节点内存跟旧A一样，CAS的操作不会有问题

#include <memory>
#include <atomic>

namespace lei {
namespace utils {

template <typename T>
class LockFreeStack {
public:
    LockFreeStack(uint32_t capacity):m_capacity(capacity) {} 
    ~LockFreeStack() = default;

    bool Push(const T &t) {
        if (m_used >= m_capacity) { return false; }
        auto p = std::make_shared<Node>();
        p->t = t;
        p->next = m_head;
        // If compare_exchange_weak return false, replace p->next with new head and continue
        while (!m_head.compare_exchange_weak(p->next, p)) {}
        m_used.fetch_add(1);
        return true;
    }

    T Pop() {
        auto p = m_head.load();
        while (p && !head.compare_exchange_weak(p, p->next)) {} 
        //if (!p) {  }
        // FIXME: empty list will crash
        if (p) { m_used.fetch_sub(1); return p->t; }
    }

    uint32_t Size() const { return m_used; } 

private:
    struct Node { 
        T t; 
        std::shared_ptr<Node> next; 
    };

    std::atomic<std::shared_ptr<Node>> m_head(nullptr);
    std::atomic<uint32_t> m_used = 0;
    uint32_t m_capacity;
};

}
}

#endif
