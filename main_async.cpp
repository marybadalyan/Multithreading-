#include "AsyncPool.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "[Main] Creating ThreadPool..." << std::endl;
    {
        MB::ThreadPool pool(4); // Create the pool in its own scope

        // Enqueue one simple task
        pool.enqueue([] { 
            std::cout << "    [Task] Hello from a task!" << std::endl; 
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "    [Task] Task finished." << std::endl; 
        });

        std::cout << "[Main] Waiting for 2 seconds before letting pool be destroyed..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

    } // <-- pool destructor ~ThreadPool() is called HERE.

    // This line will NEVER be printed.
    std::cout << "[Main] ThreadPool destroyed. Program finished." << std::endl;
    
    return 0;
}