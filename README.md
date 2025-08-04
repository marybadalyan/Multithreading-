
# Advanced C++ Thread Pool

This project is a modern C++ implementation of a thread pool, exploring several advanced concepts in concurrent programming. It begins with a classic single-queue design and evolves to demonstrate features like dynamic task production, live statistics reporting, and graceful shutdowns. The project is built using C++17 and CMake for cross-platform compatibility.

## Key Features

  - **Classic Thread Pool Design:** A robust pool of worker threads that efficiently execute tasks from a queue.
  - **Thread-Safe Task Submission:** A public `enqueue` method allows tasks (in the form of `std::function<void()>`) to be safely submitted to the pool from any thread.
  - **Graceful Shutdown:** The thread pool destructor ensures all worker threads are properly joined, preventing resource leaks.
  - **Live Statistics Reporting:** A dedicated stats-reporting thread provides a clean, periodic summary of the pool's state, including the number of active threads, pending tasks, and total tasks completed.
  - **Dynamic Task Producer:** A separate producer thread simulates a real-world workload by continuously creating and enqueueing new tasks, with a frequency that increases over time to stress-test the pool.
  - **Atomic Counters for Metrics:** Uses `std::atomic` for thread-safe tracking of performance metrics like the number of completed tasks.
  - **Cross-Platform Build:** Uses CMake to ensure the project can be easily built and run on Windows, Linux, and macOS.

## Architectural Concepts Explored

Beyond the implemented code, this project serves as a basis for exploring more advanced thread pool architecture:

  - The current implementation uses a **single, global task queue** protected by one mutex. This design is simple and provides automatic load balancing but can suffer from high lock contention on systems with many cores.

  - **Dynamic Thread Scaling:**

      - The project simulates a dynamic load, and the next logical step is a pool that dynamically adjusts its number of threads.
      - This involves a "manager" component that monitors system load (e.g., total pending tasks) and uses heuristics to decide when to add new worker threads to handle a backlog or (more complexly) remove idle threads to save resources.

## How to Build and Run

### Prerequisites

  - A C++17 compatible compiler (e.g., GCC, Clang, MSVC)
  - CMake (version 3.16 or newer)

### Build Instructions

1.  **Clone the repository:**

    ```bash
    git clone `Multithreading-`
    ```
    cd Multithreading-
    ```

2.  **Create a build directory:** It's best practice to build out-of-source.

    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake to configure the project:**

    ```bash
    cmake ..
    ```

4.  **Compile the code:**

    ```bash
    cmake --build .
    ```

### Running the Application

The executable will be created inside the `build` directory.

  - **On Windows:**
    ```powershell
    .\main.exe
    ```
  - **On Linux or macOS:**
    ```bash
    ./main
    ```

## Example Output

When you run the application, you will see a clean, periodic status report, followed by a final summary:

```
[Main] System is running. Test duration: 30 seconds.
[Stats] Active Threads: 4 | Pending Tasks: 0 | Completed Tasks: 0
[Stats] Active Threads: 4 | Pending Tasks: 12 | Completed Tasks: 35
[Stats] Active Threads: 4 | Pending Tasks: 28 | Completed Tasks: 78
[Stats] Active Threads: 4 | Pending Tasks: 45 | Completed Tasks: 121
...
[Main] Test duration over. Signaling threads to stop...
[Main] Waiting for thread pool to drain remaining tasks...
[Main] Pool is drained. Shutting down.

----------------------------------------
           FINAL REPORT
----------------------------------------
Total tasks completed: 857
Threads used in pool: 4
----------------------------------------

```

## Code Structure

  - `ThreadPool.h` / `ThreadPool.cpp`: Defines the core `ThreadPool` class, managing workers and the central task queue.
  - `main.cpp`: The main driver program. It contains the logic for the task producer and the statistics reporter and demonstrates how to use the `ThreadPool`.
  - `CMakeLists.txt`: The build configuration file for CMake.