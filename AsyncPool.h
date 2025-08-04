#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>  

namespace MB {

class ThreadPool {
public:
    ThreadPool(size_t initialThreads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    void workerLoop();

    std::atomic<bool> stop_ = false;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
    
    std::vector<std::future<void>> workerFutures_;
};

} // namespace MB