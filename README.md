
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



Of course. Creating a README file that explains *why* a certain approach fails is an excellent way to document programming concepts. This file will serve as an educational tool, demonstrating the pitfalls of using `std::async` for long-lived worker threads.

Here is a complete `README.md` file for the project that demonstrates the deadlock.

-----

## Why `std::async` is the Wrong Tool for Thread Pool Workers

### A C++ Demonstration of Deadlock and Lifetime Issues

> **Disclaimer:** The code in this repository is **intentionally flawed** for educational purposes. Its sole purpose is to demonstrate why using `std::async` to create persistent worker threads is a fundamental design error that leads to deadlocks or crashes. This is **not** a functional thread pool.

### Project Goal

This project provides a hands-on demonstration of a common C++ concurrency pitfall. By replacing the standard `std::thread` workers in a thread pool with `std::async`, we create a program that reliably **deadlocks** on shutdown. This serves to illustrate the critical differences in lifetime management and control between `std::thread` and `std::async`.

### The Core Problem: Task vs. Worker

The fundamental mistake is trying to use a **task-based abstraction** (`std::async`) for a role that requires a **long-lived worker thread**.

  - A **`std::async` task** is like a temporary contractor hired for a single job. It is expected to run a function from start to finish and then go away. Its lifetime is automatically managed by the `std::future` it returns.

  - A **`ThreadPool` worker** is like a permanent employee. It is expected to start up and then loop forever (`while (!stop)`), waiting for assignments. Its lifetime must be explicitly managed by the `ThreadPool` itself.

Using a "contractor" for a "permanent employee" role leads to critical failures in program logic, particularly during shutdown.

### Failure Mode: Deadlock on Shutdown

This project is specifically crafted to demonstrate a classic deadlock. Here is the sequence of events that causes the program to freeze:

1.  **The `std::future` Waits:** The `ThreadPool` object holds a `std::vector<std::future<void>>` for its workers. When the `ThreadPool` is destroyed at the end of `main`, the destructor for this vector is called. This, in turn, calls the destructor for each `std::future`, which **blocks and waits** for its associated worker task (`workerLoop`) to complete.

2.  **The Worker Waits:** The `workerLoop` function is trapped in its `while(!stop_)` loop. It has no work to do, so it's sleeping on a condition variable, waiting to be told either that there's a new task or that it's time to stop. It can only exit the loop when `stop_` becomes `true`.

3.  **The Destructor is Blocked:** The code responsible for setting `stop_ = true` is inside the `ThreadPool`'s destructor.

This creates a **circular dependency**:

  - The `ThreadPool` destructor is blocked, waiting for the `std::future`.
  - The `std::future` is waiting for the `workerLoop` task to finish.
  - The `workerLoop` task is waiting for the `ThreadPool` destructor to set the `stop_` flag.

Nothing can proceed, and the application freezes.

### A Note on Crashing (Use-After-Free)

While this code reliably deadlocks, a slightly different timing or implementation could lead to an even worse error: a **use-after-free crash**. This would happen if the worker threads tried to access members of the `ThreadPool` object (like its mutex) *after* the `ThreadPool`'s memory had started to be deallocated but before the worker thread was terminated. This is a severe memory corruption bug and highlights the dangers of mismatched object lifetimes.

### The Correct Approach: `std::thread` and Manual `join()`

A functional thread pool (like the one we developed previously) uses `std::thread`. Its destructor solves the deadlock by creating a safe, ordered shutdown:

1.  Explicitly set `stop_ = true`.
2.  Notify all sleeping workers to wake up.
3.  **Manually loop and `.join()` every single worker thread.** This crucial step forces the destructor to wait until all workers have safely exited their loops *before* the `ThreadPool` object and its members (like the mutexes and futures) are destroyed.

## How to Build and Observe the Failure

### Prerequisites

  - A C++17 compatible compiler (e.g., GCC, Clang, MSVC)
  - CMake (version 3.16 or newer)

### Build and Run

1.  **Create a build directory:**
    ```bash
    mkdir build && cd build
    ```
2.  **Configure and compile:**
    ```bash
    cmake ..
    cmake --build .
    ```
3.  **Run the application:**
      - On Windows: `.\main.exe`
      - On Linux/macOS: `./main`


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
    .\main.exe //or .\main_async.exe
    ```
  - **On Linux or macOS:**
    ```bash
    ./main //or ./main_async
    ```

## Example Output

When you run the application, you will see a clean, periodic status report, followed by a final summary:

.\main.exe

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

.\main_async.exe

The program will print the startup messages, execute its one task, and then attempt to shut down. The output will stop here, and the program will hang indefinitely until you manually terminate it (e.g., with `Ctrl+C`).

```
[Main] Creating ThreadPool...
[Main] Waiting for 2 seconds before letting pool be destroyed...
    [Task] Hello from a task!
    [Task] Task finished.
[Destructor] Signaling workers to stop...
[Destructor] Waiting for futures... (This is where it will freeze)
<-- PROGRAM HANGS HERE -->
```

This frozen state is the deadlock in action, clearly demonstrating why `std::thread` is the correct low-level tool for building persistent worker infrastructure in C++.

## Code Structure

  - `ThreadPool.h` / `ThreadPool.cpp`: Defines the core `ThreadPool` class, managing workers and the central task queue.
  - `main.cpp`: The main driver program. It contains the logic for the task producer and the statistics reporter and demonstrates how to use the `ThreadPool`.
  - `CMakeLists.txt`: The build configuration file for CMake.