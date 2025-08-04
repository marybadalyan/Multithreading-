// Note: DO NOT include ThreadPool.cpp
#include "ThreadPool.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <random>
#include <chrono>
#include <thread>
#include <atomic>

std::mutex g_cout_mutex; // A global mutex to protect std::cout

const size_t N = 1'000'000;
const size_t INITIAL_THREADS = 4;
const size_t MAX_THREADS = 8;

// Improved random number generation
void fill_with_random(std::vector<long long>& vec) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000);
    for (auto& val : vec) {
        val = dis(gen);
    }
}
std::function<void()> makeSumTask(
    size_t start, size_t end,
    const std::vector<long long>& numbers,
    std::vector<long long>& partialSums, // This now correctly accepts a vector of long long
    size_t resultIndex)
{
    return [start, end, &numbers, &partialSums, resultIndex] {
        // The local sum should also be long long to avoid overflow before the final assignment
        long long localSum = 0; 
        for(int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j) {
                localSum += numbers[j];
            }
        }
        partialSums[resultIndex] = localSum;
    };
}

void produceTasks(
    MB::ThreadPool& pool,
    const std::vector<long long>& numbers,
    std::vector<long long>& partialSums,
    std::atomic<bool>& stop)
{
    using namespace std::chrono_literals;

    // --- More Aggressive Parameters ---
    auto current_delay = 1000ms;       // Start faster: 1 second delay
    const auto min_delay = 10ms;         // Allow much higher frequency: 10ms
    const double decay_factor = 0.90;    // Speed up by 10% each time
    const size_t batch_size = 8;         // Enqueue 8 tasks at once!

    size_t task_counter = 0;

    std::cout << "[Producer] Starting up with aggressive settings." << std::endl;

    while (!stop) {
        // --- Enqueue a BATCH of tasks ---
        for (size_t i = 0; i < batch_size; ++i) {
            // Use modulo to cycle through the partialSums vector indices safely
            size_t resultIndex = (task_counter + i) % MAX_THREADS;
            pool.enqueue(makeSumTask(0, numbers.size(), numbers, partialSums, resultIndex));
        }
        task_counter += batch_size;
        
        {
            std::lock_guard<std::mutex> lock(g_cout_mutex);
            std::cout << "[Producer] Enqueued a batch of " << batch_size << " tasks. Next batch in "
                      << current_delay.count() << "ms. "
                      << "(Pending tasks: " << pool.getPendingTaskCount() << ")" << std::endl;
        }

        // Wait for the current delay
        std::this_thread::sleep_for(current_delay);

        // Increase the frequency for the next iteration
        current_delay *= decay_factor;
        if (current_delay < min_delay) {
            current_delay = min_delay;
        }
    }

    std::cout << "[Producer] Stop signal received. Shutting down." << std::endl;
}

int main() {
    std::cout << "Setting up..." << std::endl;
    std::vector<long long> numbers(N);
    fill_with_random(numbers);

    MB::ThreadPool pool(INITIAL_THREADS, MAX_THREADS);
    std::vector<long long> partialSums(MAX_THREADS, 0);

    // 1. Create the atomic stop flag for the producer
    std::atomic<bool> stopProducer = false;

    // 2. Launch the producer function in its own thread
    //    std::ref is needed to pass arguments by reference to a new thread
    std::thread producerThread(produceTasks,
                               std::ref(pool),
                               std::cref(numbers), // const reference
                               std::ref(partialSums),
                               std::ref(stopProducer));

    // 3. Let the simulation run for a while (e.g., 20 seconds)
    //    The main thread can do other work here, or just sleep.
    std::cout << "[Main] Producer is running. Main thread will wait for 20 seconds." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(20));

    // 4. Signal the producer to stop and wait for it to finish
    std::cout << "[Main] Signaling producer to stop..." << std::endl;
    stopProducer = true;
    producerThread.join(); // IMPORTANT: Wait for the thread to exit cleanly

    std::cout << "[Main] Producer has stopped. Waiting for thread pool to drain remaining tasks..." << std::endl;

    // Optional: wait for the pool to finish all remaining tasks
    while(pool.getPendingTaskCount() > 0) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    }
    std::this_thread::sleep_for(std::chrono::nanoseconds(100)); // Final small wait

    std::cout << "[Main] Program finished." << std::endl;
    
    // The ThreadPool's destructor will be called here, shutting down the workers.
    return 0;
}
