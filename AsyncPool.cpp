#include "AsyncPool.h"
#include <iostream>

namespace MB {

// The constructor now launches workers with std::async
ThreadPool::ThreadPool(size_t initialThreads) {
    for (size_t i = 0; i < initialThreads; ++i) {
        // We launch the worker as an async task and store its future.
        workerFutures_.push_back(
            std::async(std::launch::async, &ThreadPool::workerLoop, this)
        );
    }
}

// The destructor will now cause a deadlock.
ThreadPool::~ThreadPool() {
    std::cout << "[Destructor] Signaling workers to stop..." << std::endl;
    stop_ = true;
    condition_.notify_all();

    // The manual join loop is gone.
    // NOW, when this function exits, the destructor for workerFutures_ will be called.
    // That destructor will block on the first future, waiting for its workerLoop
    // to finish. But the workerLoop is waiting for stop_ to be true, which we've
    // already set. The problem is, it might be stuck waiting on the condition
    // variable, and the sequence of events leads to a freeze.
    std::cout << "[Destructor] Waiting for futures... (This is where it will freeze)" << std::endl;
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty()) {
                return; // Exit loop
            }

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        tasks_.push(std::move(task));
    }
    condition_.notify_one();
}

} // namespace MB