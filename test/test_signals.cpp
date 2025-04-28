#include "../include/signal.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <cstring>

// Signal handler for simple signals
void simpleHandler(int signum)
{
    std::cout << "Simple handler called for signal: " << signum << std::endl;
}

// Extended signal handler
void extendedHandler(int signum, SigInfo *info, void *context)
{
    std::cout << "Extended handler called for signal: " << signum << std::endl;
    if (info != nullptr)
    {
        std::cout << "  Signal number: " << info->si_signo << std::endl;
        std::cout << "  Signal code: " << info->si_code << std::endl;
        std::cout << "  Sending process ID: " << info->si_pid << std::endl;
    }
}

// Windows-compatible pause implementation
#ifdef _WIN32
void win_pause_with_timeout()
{
    // For Windows, we'll use a timed sleep instead of pause
    std::this_thread::sleep_for(std::chrono::seconds(5));
}
#endif

int main()
{
    std::cout << "Signal Functionality Test" << std::endl;
    std::cout << "Process ID: " << getpid() << std::endl;

    // Test 1: Register a simple signal handler
    std::cout << "\nTest 1: Register a simple signal handler for SIGUSR1" << std::endl;

    SigAction act1;
    act1.sa_handler = simpleHandler;
    act1.sa_sigaction = nullptr;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = 0;
    act1.sa_restorer = nullptr;

    if (sigaction(SIGUSR1, &act1, nullptr) < 0)
    {
        std::cerr << "Failed to register simple signal handler" << std::endl;
        return 1;
    }

    std::cout << "Simple handler registered for SIGUSR1" << std::endl;

    // Test 2: Register an extended signal handler
    std::cout << "\nTest 2: Register an extended signal handler for SIGUSR2" << std::endl;

    SigAction act2;
    act2.sa_handler = nullptr;
    act2.sa_sigaction = extendedHandler;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = 0;
    act2.sa_restorer = nullptr;

    if (sigaction(SIGUSR2, &act2, nullptr) < 0)
    {
        std::cerr << "Failed to register extended signal handler" << std::endl;
        return 1;
    }

    std::cout << "Extended handler registered for SIGUSR2" << std::endl;

    // Test 3: Block/unblock signals
    std::cout << "\nTest 3: Test signal blocking/unblocking" << std::endl;

    SigSet blockSet;
    blockSet.resize(SIGMAX + 1, false);
    sigaddset(&blockSet, SIGUSR1);

    std::cout << "Blocking SIGUSR1..." << std::endl;
    sigprocmask(0, &blockSet, nullptr);

    // Test 4: Send signal to self
    std::cout << "\nTest 4: Sending SIGUSR1 to self (should be blocked)" << std::endl;
    kill(getpid(), SIGUSR1);
    std::cout << "SIGUSR1 sent to self (should be pending)" << std::endl;

    std::cout << "Unblocking SIGUSR1..." << std::endl;
    SigSet unblockSet;
    unblockSet.resize(SIGMAX + 1, false);
    sigprocmask(0, &unblockSet, nullptr);
    std::cout << "SIGUSR1 unblocked (handler should be called now)" << std::endl;

    // Test 5: Test the pause functionality
    std::cout << "\nTest 5: Test pause() functionality" << std::endl;
    std::cout << "Send SIGUSR2 signal to test pause using: kill -" << SIGUSR2 << " " << getpid() << std::endl;
    std::cout << "Pausing until signal received..." << std::endl;

    // Create a thread to send the signal after a delay
    std::thread([pid = getpid()]()
                {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "Sending SIGUSR2 to process from thread..." << std::endl;
        kill(pid, SIGUSR2); })
        .detach();

#ifdef _WIN32
    // Windows doesn't have a real pause, so we'll use a timed wait
    win_pause_with_timeout();
#else
    pause();
#endif

    std::cout << "Woke up from pause!" << std::endl;

    // Test 6: Test signal handler chaining
    std::cout << "\nTest 6: Test signal handler replacement" << std::endl;

    SigAction oldact;
    SigAction newact;
    newact.sa_handler = [](int signum)
    {
        std::cout << "New handler called for signal: " << signum << std::endl;
    };
    newact.sa_sigaction = nullptr;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    newact.sa_restorer = nullptr;

    if (sigaction(SIGUSR1, &newact, &oldact) < 0)
    {
        std::cerr << "Failed to register new signal handler" << std::endl;
        return 1;
    }

    std::cout << "New handler registered for SIGUSR1" << std::endl;
    std::cout << "Old handler address: " << (void *)oldact.sa_handler << std::endl;

    // Test 7: Send signal to self with new handler
    std::cout << "\nTest 7: Sending SIGUSR1 to self (should call new handler)" << std::endl;
    kill(getpid(), SIGUSR1);

    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}