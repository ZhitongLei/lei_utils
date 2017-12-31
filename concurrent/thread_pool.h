#ifndef __LEI_UTILS_THREAD_H__
#define __LEI_UTILS_THREAD_H__

#include <mutex>
#include <thread>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>

namespace lei_utils {
namespace concurrent {

class ThreadPool {
public:
    ThreadPool(uint32_t thread_num, uint32_t max_task);
    ~ThreadPool();

    ThreadPool(const ThreadPool &rhs) = delete;
    ThreadPool& operator=(const ThreadPool &rhs) = delete;

    using Task = std::function<void()>;
    void Execute(const Task &task);

    void Stop();
    bool IsFull();

private:    
    void WorkerThread();

    std::vector<std::thread> m_workers;
    uint32_t m_max_task;
    bool m_running;

    std::mutex m_task_mutex;
    std::deque<Task> m_tasks;

    std::condition_variable m_not_empty_cond;
    std::condition_variable m_not_full_cond;
};

} // end of concurrent
} // end of lei_utils

#endif
