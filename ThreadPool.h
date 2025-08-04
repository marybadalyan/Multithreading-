#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace MB {

class ThreadPool {
public:
    ThreadPool(size_t initialThreads, size_t maxThreads);
    ~ThreadPool();

    // Deleted copy and move constructors for simplicity
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void enqueue(std::function<void()> task);

    // Safe way to get stats
    size_t getThreadCount() const;
    size_t getPendingTaskCount() const;

private:
    void addThread();
    void workerLoop(); // The main loop for each worker thread

    size_t maxThreads;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop = false;
};

} // namespace MB