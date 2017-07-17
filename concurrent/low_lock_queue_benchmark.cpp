#include <benchmark/benchmark.h>
#include <stdio.h>
#include <string>
#include <assert.h>
#include <thread>
#include <atomic>
#include "low_lock_queue.h"
#include "lock_queue.h"

using namespace lei::utils;
using namespace std;

struct UserInfo {
    UserInfo() = default;

    UserInfo(uint32_t id, const string &name)
        : id_(id), name_(name) {}
    uint32_t id_;
    string name_;
};

void BaseTest() {
    LowLockQueue<UserInfo> queue(1000);
    size_t size = sizeof(queue);
    printf("sizeof(queue) = %zu\n", size);

    UserInfo user(2323, "ff\0aadsfs");
    queue.EnQueue(user);
    assert(queue.size() == 1);

    UserInfo u2;
    queue.DeQueue(u2);
    assert(queue.size() == 0);
    printf("u2, id=%u name=%s\n", u2.id_, u2.name_.data());
}

LockQueue<UserInfo> lock_queue(100000);
UserInfo user(3243248, "This is the user info. Benchmark go.....");
UserInfo u;
atomic<double> enqueue_count(0);
atomic<double> dequeue_count(0);
atomic<uint64_t> processed_items(0);
static void BM_lock_queue(benchmark::State& state) {
    //int threads = state.threads;
    //int write_threads = threads / 3;
    if (state.thread_index == 0) {
        enqueue_count = 0;
        dequeue_count = 0;
        processed_items = 0;
    }

    while (state.KeepRunning()) {
        if (state.thread_index % 2 == 0) {
            if (lock_queue.EnQueue(user)) {
                enqueue_count = enqueue_count + 1.0;
            }
        } else {                
        //} else if (state.thread_index % 2 == 1) {
            if (lock_queue.DeQueue(u)) {
                processed_items.fetch_add(1);
                dequeue_count = dequeue_count + 1.0;
            }
        }
    }

    if (state.thread_index == 0) {
        state.counters["Enqueue"] = enqueue_count.load();
        state.counters["Dequeue"] = dequeue_count.load();
        state.SetItemsProcessed(processed_items.load());
    }
}

LowLockQueue<UserInfo> low_lock_queue(100000);
static void BM_low_lock_queue(benchmark::State& state) {
    if (state.thread_index == 0) {
        enqueue_count = 0;
        dequeue_count = 0;
        processed_items = 0;
    }

    while (state.KeepRunning()) {
        if (state.thread_index % 2 == 0) {
            if (low_lock_queue.EnQueue(user)) {
                enqueue_count = enqueue_count + 1.0;
            }
        } else {                
        //} else if (state.thread_index % 2 == 1) {
            if (low_lock_queue.DeQueue(u)) {
                processed_items.fetch_add(1);
                dequeue_count = dequeue_count + 1.0;
            }
        }
    }

    if (state.thread_index == 0) {
        state.counters["Enqueue"] = enqueue_count.load();
        state.counters["Dequeue"] = dequeue_count.load();
        state.SetItemsProcessed(processed_items.load());
    }
}

BENCHMARK(BM_lock_queue)->Threads(10)->UseRealTime();
BENCHMARK(BM_low_lock_queue)->Threads(10)->UseRealTime();
BENCHMARK_MAIN();
