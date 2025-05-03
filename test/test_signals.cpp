#include "../include/signal.h"
#include "../include/process.h"
#include "../include/kernel.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <functional>

// Signal names for better readability
const std::map<int, std::string> SIGNAL_NAMES = {
    {SIGHUP, "SIGHUP"},   // 1: Hangup
    {SIGINT, "SIGINT"},   // 2: Interrupt (Ctrl+C)
    {SIGQUIT, "SIGQUIT"}, // 3: Quit (Ctrl+\)
    {SIGILL, "SIGILL"},   // 4: Illegal instruction
    {SIGTRAP, "SIGTRAP"}, // 5: Trace/breakpoint trap
    {SIGABRT, "SIGABRT"}, // 6: Abort
    {SIGKILL, "SIGKILL"}, // 9: Kill (cannot be caught)
    {SIGUSR1, "SIGUSR1"}, // 10: User-defined signal 1
    {SIGUSR2, "SIGUSR2"}, // 11: User-defined signal 2
    {SIGALRM, "SIGALRM"}, // 14: Alarm clock
    {SIGTERM, "SIGTERM"}  // 15: Termination
};

// Global variables for signal tracking
namespace {
    volatile bool signal_received = false;
    volatile int last_signal_received = 0;
    volatile int signal_count = 0;
}

// Test case structure
struct TestCase {
    std::string name;
    std::function<bool()> test;
    std::string description;
};

// Basic signal handler that just records the signal
void basicSignalHandler(int signum) {
    signal_received = true;
    last_signal_received = signum;
    signal_count++;
    std::cout << "Basic handler received signal: " << SIGNAL_NAMES.at(signum) << " (" << signum << ")" << std::endl;
}

// Extended signal handler that shows signal info
void extendedSignalHandler(int signum, SigInfo* info, void*) {
    signal_received = true;
    last_signal_received = signum;
    signal_count++;
    std::cout << "Extended handler received:\n"
              << "  Signal: " << SIGNAL_NAMES.at(signum) << " (" << signum << ")\n"
              << "  Sender PID: " << info->si_pid << "\n"
              << "  Signal code: " << info->si_code << std::endl;
}

// Reset global signal tracking variables
void resetSignalTracking() {
    signal_received = false;
    last_signal_received = 0;
    signal_count = 0;
}

// Test a specific signal with debugging
bool testSignalWithDebug(int signum, bool useExtendedHandler = false) {
    std::cout << "\nTesting " << SIGNAL_NAMES.at(signum) << " with "
              << (useExtendedHandler ? "extended" : "basic") << " handler..." << std::endl;
    
    resetSignalTracking();

    // Create a process for testing
    Process* process = new Process(1);

    // Skip handler registration for SIGKILL as it cannot be caught
    if (signum != SIGKILL) {
        SigAction act;
        if (useExtendedHandler) {
            act.sa_handler = nullptr;
            act.sa_sigaction = extendedSignalHandler;
        } else {
            act.sa_handler = basicSignalHandler;
            act.sa_sigaction = nullptr;
        }
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        act.sa_restorer = nullptr;

        std::cout << "Registering handler..." << std::endl;
        int result = process->registerSignalAction(signum, &act, nullptr);
        if (result < 0) {
            std::cout << "Failed to register handler (result = " << result << ")" << std::endl;
            delete process;
            return false;
        }
        std::cout << "Handler registered successfully" << std::endl;
    }

    // Start the process
    process->start([]() {
        // Just sleep to keep the process running
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    // Send the signal to the process
    std::cout << "Sending " << SIGNAL_NAMES.at(signum) << " to process " << process->getPid() << "..." << std::endl;
    
    int result = process->sendSignal(signum);
    if (result < 0) {
        std::cout << "Failed to send signal (result = " << result << ")" << std::endl;
        delete process;
        return false;
    }
    std::cout << "Signal sent successfully" << std::endl;

    // Wait for signal processing
    std::cout << "Waiting for signal processing..." << std::endl;
    for (int i = 0; i < 10 && !signal_received; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "." << std::flush;
    }
    std::cout << std::endl;

    // Verify signal receipt (except for SIGKILL which terminates the process)
    if (signum != SIGKILL) {
        std::cout << "Checking signal receipt..." << std::endl;
        std::cout << "  signal_received = " << signal_received << std::endl;
        std::cout << "  last_signal_received = " << last_signal_received << std::endl;
        std::cout << "  signal_count = " << signal_count << std::endl;

        bool success = signal_received && last_signal_received == signum;
        if (success) {
            std::cout << SIGNAL_NAMES.at(signum) << " test passed!" << std::endl;
        } else {
            std::cout << SIGNAL_NAMES.at(signum) << " test failed!" << std::endl;
        }

        delete process;
        return success;
    }

    delete process;
    return true;
}

int main() {
    std::cout << "Signal Handling Test Suite" << std::endl;
    std::cout << "==========================" << std::endl;

    // Initialize kernel
    Kernel* kernel = Kernel::getInstance();
    std::cout << "Kernel initialized" << std::endl;

    // Test cases
    struct TestCase {
        std::string name;
        std::function<bool()> test;
        std::string description;
    };

    std::vector<TestCase> tests = {
        {
            "Basic Signal Handling",
            []() { return testSignalWithDebug(SIGUSR1, false); },
            "Tests registration and handling of basic signals"
        },
        {
            "Extended Signal Handling",
            []() { return testSignalWithDebug(SIGUSR2, true); },
            "Tests extended signal handling with additional information"
        },
        {
            "Signal Blocking",
            []() {
                std::cout << "\nTesting signal blocking..." << std::endl;
                resetSignalTracking();

                // Create a process for testing
                Process* process = new Process(1);

                // Register handler for SIGUSR1
                SigAction act;
                act.sa_handler = basicSignalHandler;
                act.sa_sigaction = nullptr;
                sigemptyset(&act.sa_mask);
                act.sa_flags = 0;
                act.sa_restorer = nullptr;

                std::cout << "Registering handler..." << std::endl;
                if (process->registerSignalAction(SIGUSR1, &act, nullptr) < 0) {
                    std::cout << "Failed to register handler" << std::endl;
                    delete process;
                    return false;
                }
                std::cout << "Handler registered successfully" << std::endl;

                // Start the process
                process->start([]() {
                    // Just sleep to keep the process running
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                });

                // Block SIGUSR1
                std::cout << "Blocking SIGUSR1..." << std::endl;
                SigSet blockSet;
                blockSet.resize(SIGMAX + 1, false);
                sigaddset(&blockSet, SIGUSR1);
                process->setBlockedSignals(blockSet);

                // Send blocked signal
                std::cout << "Sending blocked signal..." << std::endl;
                process->sendSignal(SIGUSR1);

                // Wait to ensure signal is blocked
                std::cout << "Waiting to verify signal is blocked..." << std::endl;
                for (int i = 0; i < 5; i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    std::cout << "." << std::flush;
                }
                std::cout << std::endl;

                bool wasBlocked = !signal_received;
                std::cout << "Signal was " << (wasBlocked ? "blocked" : "not blocked") << std::endl;

                // Unblock signal
                std::cout << "Unblocking signal..." << std::endl;
                SigSet unblockSet;
                unblockSet.resize(SIGMAX + 1, false);
                process->setBlockedSignals(unblockSet);

                // Wait for signal to be delivered
                std::cout << "Waiting for signal delivery..." << std::endl;
                for (int i = 0; i < 10 && !signal_received; i++) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    std::cout << "." << std::flush;
                }
                std::cout << std::endl;

                std::cout << "Checking final state..." << std::endl;
                std::cout << "  signal_received = " << signal_received << std::endl;
                std::cout << "  last_signal_received = " << last_signal_received << std::endl;

                bool success = wasBlocked && signal_received && last_signal_received == SIGUSR1;
                delete process;
                return success;
            },
            "Tests blocking and unblocking of signals"
        },
        {
            "SIGKILL Behavior",
            []() { return testSignalWithDebug(SIGKILL); },
            "Tests special behavior of SIGKILL"
        }
    };

    // Run tests
    int passed = 0;
    for (const auto& test : tests) {
        std::cout << "\nRunning: " << test.name << std::endl;
        std::cout << "Description: " << test.description << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        
        bool result = test.test();
        if (result) {
            std::cout << "✓ " << test.name << " PASSED" << std::endl;
            passed++;
        } else {
            std::cout << "✗ " << test.name << " FAILED" << std::endl;
        }
    }

    // Print summary
    std::cout << "\nTest Summary" << std::endl;
    std::cout << "============" << std::endl;
    std::cout << "Passed: " << passed << "/" << tests.size() << " tests" << std::endl;

    return 0;
}

