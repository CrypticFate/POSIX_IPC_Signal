#include "../include/signal.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

#ifdef _WIN32
// Windows-specific implementation
void runChildProcess()
{
    std::cout << "Child process simulation started with PID: " << getpid() << std::endl;

    // Register a signal handler for SIGUSR1
    SigAction act;
    act.sa_handler = [](int signum)
    {
        std::cout << "Child received signal: " << signum << std::endl;
    };
    act.sa_sigaction = nullptr;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_restorer = nullptr;

    if (sigaction(SIGUSR1, &act, nullptr) < 0)
    {
        std::cerr << "Failed to register signal handler" << std::endl;
        return;
    }

    // Simulate child process behavior
    for (int i = 0; i < 3; i++)
    {
        std::cout << "Child process running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Child process exiting" << std::endl;
}
#endif

int main()
{
    // This test creates a child process and then kills it with different signals
    std::cout << "Testing kill functionality..." << std::endl;

#ifdef _WIN32
    // Windows implementation using threads instead of fork
    std::cout << "Parent process with PID: " << getpid() << std::endl;
    std::cout << "Creating simulated child process..." << std::endl;

    // Simulate child PID
    int childPid = getpid() + 1;
    std::cout << "Simulated child process with PID: " << childPid << std::endl;

    // Create a thread to simulate the child process
    std::thread childThread(runChildProcess);
    childThread.detach();

    // Wait a moment for child to set up
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Simulate sending signals
    std::cout << "Simulating sending SIGUSR1 to child..." << std::endl;
    std::cout << "Simulating sending SIGKILL to child..." << std::endl;

    std::cout << "Parent process exiting (Windows simulation)" << std::endl;
#else
    // Original POSIX implementation
    // Fork a child process
    pid_t childPid = fork();

    if (childPid == -1)
    {
        std::cerr << "Failed to fork child process" << std::endl;
        return 1;
    }

    if (childPid == 0)
    {
        // Child process
        std::cout << "Child process started with PID: " << getpid() << std::endl;

        // Register a signal handler for SIGUSR1
        SigAction act;
        act.sa_handler = [](int signum)
        {
            std::cout << "Child received signal: " << signum << std::endl;
        };
        act.sa_sigaction = nullptr;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_restorer = nullptr;

        if (sigaction(SIGUSR1, &act, nullptr) < 0)
        {
            std::cerr << "Failed to register signal handler" << std::endl;
            return 1;
        }

        // Simply wait for signals until terminated
        while (true)
        {
            pause();
            std::cout << "Child woke up from pause" << std::endl;
        }

        return 0;
    }
    else
    {
        // Parent process
        std::cout << "Parent process with PID: " << getpid() << std::endl;
        std::cout << "Created child process with PID: " << childPid << std::endl;

        // Wait a moment for child to set up
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // First, send SIGUSR1 to the child
        std::cout << "Sending SIGUSR1 to child..." << std::endl;
        if (kill(childPid, SIGUSR1) < 0)
        {
            std::cerr << "Failed to send SIGUSR1 to child" << std::endl;
        }

        // Wait a moment
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Finally, send SIGKILL to terminate the child
        std::cout << "Sending SIGKILL to child..." << std::endl;
        if (kill(childPid, SIGKILL) < 0)
        {
            std::cerr << "Failed to send SIGKILL to child" << std::endl;
        }

        std::cout << "Parent process exiting" << std::endl;
    }
#endif

    return 0;
}