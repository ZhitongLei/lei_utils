#include "thread_pool.h"
#include <assert.h>

namespace lei_utils {
namespace concurrent {

ThreadPool::ThreadPool(uint32_t thread_num, uint32_t max_task)
    : m_max_task(max_task) {
    assert(thread_num > 0);
    m_workers.reserve(thread_num);
    for (uint32_t i = 0; i < thread_num; i++) {
        m_workers.push_back(std::thread(&ThreadPool::WorkerThread, this));
    }
    m_running = true;
}

ThreadPool::~ThreadPool() {
    if (m_running) { Stop(); }
}

void ThreadPool::Stop() {
    if (!m_running) { return; }

    {
        std::lock_guard<std::mutex> lock(m_task_mutex);
        m_running = false;
        m_not_empty_cond.notify_all();
    }
    for (auto &w : m_workers) { w.join(); }
}

bool ThreadPool::IsFull() {
    std::lock_guard<std::mutex> lock(m_task_mutex);
    return m_tasks.size() >= m_max_task;
}

void ThreadPool::Execute(const Task &task) {
    std::unique_lock<std::mutex> lock(m_task_mutex);
    m_not_full_cond.wait(lock, [this]{return m_tasks.size() < m_max_task;});
    m_tasks.push_back(task);
    m_not_empty_cond.notify_one();
}

void ThreadPool::WorkerThread() {
    while (m_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(m_task_mutex);
            if (m_tasks.empty()) { m_not_empty_cond.wait(lock); }
            if (!m_tasks.empty()) {
                task = std::move(m_tasks.front());
                m_tasks.pop_front();
            }
            m_not_full_cond.notify_one();
        }
        if (task) { task(); }
    }
}

} // end of concurrent
} // end of lei_utils
