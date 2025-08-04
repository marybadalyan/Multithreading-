# Specify the minimum version of CMake required.
cmake_minimum_required(VERSION 3.16)

# Set the project name and specify that it's a C++ project.
project(ThreadPoolExample CXX)

# Set the C++ standard to C++17 (your code uses features from it).
# This is the modern way to set the standard for a target.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This command finds the system's thread library. It's necessary because
# you are using std::thread.
find_package(Threads REQUIRED)

# This is the key command. It tells CMake to create an executable named
# "main" from the specified source files.
# This replaces the need for #include "ThreadPool.cpp".
add_executable(main
    main.cpp
    ThreadPool.cpp
)

add_executable(main_async
    main_async.cpp
    AsyncPool.cpp
)
# Link the executable against the threads library found earlier.
# The Threads::Threads part is a modern CMake "target" that works across
# different platforms (Linux, macOS, Windows).
target_link_libraries(main PRIVATE Threads::Threads)