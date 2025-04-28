#include "../include/signal.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

// Simple handler for signals
void signalHandler(int signum)
{
    std::cout << "Signal " << signum << " received by handler!" << std::endl;
}

// Windows-specific pause implementation
#ifdef _WIN32
void win_pause()
{
    // For Windows, we'll use a brief sleep instead of a real pause
    while (true)
    {
        std::cout << "Waiting for signal (Windows simulation)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
#endif

int main()
{
    // Register signal handler for SIGUSR1
    SigAction act;
    act.sa_handler = signalHandler;
    act.sa_sigaction = nullptr;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_restorer = nullptr;

    if (sigaction(SIGUSR1, &act, nullptr) < 0)
    {
        std::cerr << "Failed to register signal handler" << std::endl;
        return 1;
    }

    std::cout << "Signal handler registered for SIGUSR1" << std::endl;
    std::cout << "Process ID: " << getpid() << std::endl;
    std::cout << "Send a SIGUSR1 signal using: kill -10 " << getpid() << std::endl;

    // Wait for signals
    while (true)
    {
        std::cout << "Waiting for signal..." << std::endl;
#ifdef _WIN32
        // Simulate signal handling on Windows
        std::this_thread::sleep_for(std::chrono::seconds(2));
#else
        pause();
#endif
        std::cout << "Woke up from pause" << std::endl;
    }

    return 0;
}