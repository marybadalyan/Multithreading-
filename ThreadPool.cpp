#include "ThreadPool.h"

namespace MB {

ThreadPool::ThreadPool(size_t initialThreads, size_t maxThreads)
    : maxThreads(maxThreads) {
    for (size_t i = 0; i < initialThreads; ++i) {
        addThread();
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });

            if (stop && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        }
        // Execute the task outside the lock
        task();
    }
}

void ThreadPool::addThread() {
    // Lock to safely modify the workers vector
    std::unique_lock<std::mutex> lock(queueMutex);
    if (workers.size() < maxThreads) {
        workers.emplace_back([this] { this->workerLoop(); });
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(std::move(task));
    }
    condition.notify_one();
}

size_t ThreadPool::getThreadCount() const {
    std::unique_lock<std::mutex> lock(queueMutex);
    return workers.size();
}

size_t ThreadPool::getPendingTaskCount() const {
    std::unique_lock<std::mutex> lock(queueMutex);
    return tasks.size();
}

} // namespace MB