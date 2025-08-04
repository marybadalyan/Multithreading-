#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

// --- Global constants and variables ---
const size_t N = 1'000'000;
const size_t INITIAL_THREADS = 4;
const size_t MAX_THREADS = 8;
std::mutex g_cout_mutex;

// --- Forward Declarations ---
void produceTasks(MB::ThreadPool&, std::atomic<size_t>&, std::atomic<bool>&);
void printStats(MB::ThreadPool&, std::atomic<size_t>&, std::atomic<bool>&);


// 1. THE TASK IS NOW SIMPLER AND INCREMENTS A COUNTER
std::function<void()> makeHeavyTask(std::atomic<size_t>& tasksCompleted)
{
    return [&tasksCompleted] {
        // This task does a predictable amount of work.
        // It's heavy enough to take time, but not infinite.
        volatile double result = 0.0;
        for (size_t j = 0; j < N; ++j) {
            result += 3.14159 / (double)(j + 1);
        }

        // Atomically increment the completed task counter.
        tasksCompleted++;
    };
}

// 2. THE PRODUCER IS NOW "QUIET" - It no longer prints to the console.
void produceTasks(MB::ThreadPool& pool, std::atomic<size_t>& tasksCompleted, std::atomic<bool>& stop)
{
    using namespace std::chrono_literals;
    auto current_delay = 1000ms;
    const auto min_delay = 50ms;
    const double decay_factor = 0.90;
    const size_t batch_size = 8;

    while (!stop) {
        for (size_t i = 0; i < batch_size; ++i) {
            pool.enqueue(makeHeavyTask(tasksCompleted));
        }
        std::this_thread::sleep_for(current_delay);
        current_delay *= decay_factor;
        if (current_delay < min_delay) {
            current_delay = min_delay;
        }
    }
}

// 3. NEW STATS-PRINTING FUNCTION
// This runs on its own thread and prints a clean summary periodically.
void printStats(MB::ThreadPool& pool, std::atomic<size_t>& tasksCompleted, std::atomic<bool>& stop) {
    using namespace std::chrono_literals;
    while (!stop) {
        std::this_thread::sleep_for(2s); // Print stats every 2 seconds

        // Lock once to print a clean, single line.
        std::lock_guard<std::mutex> lock(g_cout_mutex);
        std::cout << "[Stats] Active Threads: " << pool.getThreadCount()
                  << " | Pending Tasks: " << pool.getPendingTaskCount()
                  << " | Completed Tasks: " << tasksCompleted.load() << std::endl;
    }
}

// 4. MAIN IS UPDATED to manage the new stats thread and print the final report.
int main() {
    MB::ThreadPool pool(INITIAL_THREADS, MAX_THREADS);

    std::atomic<size_t> tasksCompleted = 0;
    std::atomic<bool> stopAll = false;

    // Launch the producer thread
    std::thread producerThread(produceTasks, std::ref(pool), std::ref(tasksCompleted), std::ref(stopAll));

    // Launch the new stats-printing thread
    std::thread statsThread(printStats, std::ref(pool), std::ref(tasksCompleted), std::ref(stopAll));

    std::cout << "[Main] System is running. Test duration: 30 seconds." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // --- Shutdown sequence ---
    std::cout << "[Main] Test duration over. Signaling threads to stop..." << std::endl;
    stopAll = true;
    producerThread.join();
    statsThread.join();

    std::cout << "[Main] Waiting for thread pool to drain remaining tasks..." << std::endl;
    while(pool.getPendingTaskCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "[Main] Pool is drained. Shutting down." << std::endl;
    
    // --- Final Report ---
    std::cout << "\n----------------------------------------" << std::endl;
    std::cout << "           FINAL REPORT" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total tasks completed: " << tasksCompleted.load() << std::endl;
    std::cout << "Threads used in pool: " << pool.getThreadCount() << std::endl;
    std::cout << "----------------------------------------\n" << std::endl;
    
    return 0;
}