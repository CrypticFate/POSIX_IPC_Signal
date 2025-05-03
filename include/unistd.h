#ifndef UNISTD_H
#define UNISTD_H

#include <process.h>
// #include <windows.h>
#include "signal.h"
#include "kernel.h"
#include <iostream>
#include <thread>
#include <chrono>

// POSIX functions we need to implement
typedef int pid_t;

inline pid_t getpid()
{
    return Kernel::getInstance()->getCurrentPid();
}

// Simple fork implementation for Windows
// Note: This is a simplified version that doesn't fully replicate POSIX fork
inline pid_t fork()
{
    std::cerr << "Warning: fork() is not fully supported on Windows\n";
    // Return a fake child PID for testing
    return getpid() + 1;
}

// Pause implementation for Windows
inline int pause()
{
    // Windows doesn't have a direct equivalent to pause()
    // This is a simplistic implementation
    std::cout << "Windows pause() simulation - sleeping for 2 seconds\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}

#endif // UNISTD_H