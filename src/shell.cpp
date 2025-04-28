#include "../include/shell.h"
#include "../include/kernel.h"
#include "../include/signal.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Shell::Shell() : running(false)
{
}

Shell::~Shell()
{
    stop();
}

void Shell::run()
{
    running = true;
    std::string command;

    std::cout << "Signal OS Shell" << std::endl;
    std::cout << "Type 'help' for available commands" << std::endl;

    while (running)
    {
        std::cout << "> ";
        std::getline(std::cin, command);

        if (command.empty())
        {
            continue;
        }

        std::vector<std::string> args = parseCommand(command);
        if (args.empty())
        {
            continue;
        }

        const std::string &cmd = args[0];

        if (cmd == "help")
        {
            cmdHelp(args);
        }
        else if (cmd == "kill")
        {
            cmdKill(args);
        }
        else if (cmd == "ps")
        {
            cmdPs(args);
        }
        else if (cmd == "run")
        {
            cmdRun(args);
        }
        else if (cmd == "exit" || cmd == "quit")
        {
            cmdExit(args);
        }
        else
        {
            std::cout << "Unknown command: " << cmd << std::endl;
            std::cout << "Type 'help' for available commands" << std::endl;
        }
    }
}

void Shell::stop()
{
    running = false;
}

std::vector<std::string> Shell::parseCommand(const std::string &cmd)
{
    std::vector<std::string> args;
    std::stringstream ss(cmd);
    std::string arg;

    while (ss >> arg)
    {
        args.push_back(arg);
    }

    return args;
}

void Shell::cmdHelp(const std::vector<std::string> &args)
{
    std::cout << "Available commands:" << std::endl;
    std::cout << "  help             - Display this help message" << std::endl;
    std::cout << "  kill [-NUM] PID  - Send signal NUM to process PID" << std::endl;
    std::cout << "  ps               - List all processes" << std::endl;
    std::cout << "  run TEST         - Run a test program" << std::endl;
    std::cout << "  exit, quit       - Exit the shell" << std::endl;
}

void Shell::cmdKill(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cout << "Usage: kill [-NUM] PID" << std::endl;
        return;
    }

    int signum = SIGTERM; // Default signal
    int pid;

    if (args.size() >= 3 && args[1][0] == '-')
    {
        // Parse signal number from -NUM format
        try
        {
            signum = std::stoi(args[1].substr(1));
            pid = std::stoi(args[2]);
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid arguments" << std::endl;
            return;
        }
    }
    else
    {
        // Only PID provided, use default signal
        try
        {
            pid = std::stoi(args[1]);
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid PID" << std::endl;
            return;
        }
    }

    int result = kill(pid, signum);
    if (result < 0)
    {
        std::cout << "Failed to send signal " << signum << " to process " << pid << std::endl;
    }
    else
    {
        std::cout << "Signal " << signum << " sent to process " << pid << std::endl;
    }
}

void Shell::cmdPs(const std::vector<std::string> &args)
{
    // In a real implementation, this would query the kernel for process information
    std::cout << "PID\tSTATE\tCOMMAND" << std::endl;

    // For demonstration only
    std::cout << "1\tR\tSystem Process" << std::endl;

    // Get processes from kernel would be implemented here
}

void Shell::cmdRun(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cout << "Usage: run TEST" << std::endl;
        std::cout << "Available tests:" << std::endl;
        std::cout << "  trap    - Test signal trap functionality" << std::endl;
        std::cout << "  kill    - Test kill functionality" << std::endl;
        std::cout << "  signals - Test various signal behaviors" << std::endl;
        return;
    }

    const std::string &testName = args[1];

    if (testName == "trap")
    {
        // Test signal trapping functionality
        std::cout << "Running signal trap test..." << std::endl;

        // Create a process that traps signals
        auto trapTest = []()
        {
            // Define a signal handler
            auto handler = [](int signum)
            {
                std::cout << "Signal handler called for signal " << signum << std::endl;
            };

            // Register the handler for SIGUSR1
            SigAction act;
            act.sa_handler = handler;
            act.sa_sigaction = nullptr;
            sigemptyset(&act.sa_mask);
            act.sa_flags = 0;
            act.sa_restorer = nullptr;

            if (sigaction(SIGUSR1, &act, nullptr) < 0)
            {
                std::cout << "Failed to register signal handler" << std::endl;
                return;
            }

            std::cout << "Signal handler registered for SIGUSR1" << std::endl;
            std::cout << "Process ID: " << Kernel::getInstance()->getCurrentPid() << std::endl;
            std::cout << "Send a SIGUSR1 signal to this process to trigger the handler" << std::endl;
            std::cout << "Press Ctrl+C to exit" << std::endl;

            // Wait for signals
            while (true)
            {
                pause();
            }
        };

        int pid = Kernel::getInstance()->createProcess(trapTest);
        std::cout << "Trap test process created with PID " << pid << std::endl;
    }
    else if (testName == "kill")
    {
        // Test kill functionality
        std::cout << "Running kill test..." << std::endl;

        // Create a process that can be killed
        auto killTest = []()
        {
            std::cout << "Process ID: " << Kernel::getInstance()->getCurrentPid() << std::endl;
            std::cout << "Send SIGKILL to terminate this process" << std::endl;

            // Wait for signals
            while (true)
            {
                pause();
                std::cout << "Woke up from pause" << std::endl;
            }
        };

        int pid = Kernel::getInstance()->createProcess(killTest);
        std::cout << "Kill test process created with PID " << pid << std::endl;
    }
    else if (testName == "signals")
    {
        // Test various signal behaviors
        std::cout << "Running signal behavior test..." << std::endl;

        // Create a process that tests various signal behaviors
        auto signalTest = []()
        {
            // Define signal handlers
            auto handler1 = [](int signum)
            {
                std::cout << "Handler 1 called for signal " << signum << std::endl;
            };

            auto handler2 = [](int signum)
            {
                std::cout << "Handler 2 called for signal " << signum << std::endl;
            };

            // Register handlers for different signals
            SigAction act1, act2;

            act1.sa_handler = handler1;
            act1.sa_sigaction = nullptr;
            sigemptyset(&act1.sa_mask);
            act1.sa_flags = 0;
            act1.sa_restorer = nullptr;

            act2.sa_handler = handler2;
            act2.sa_sigaction = nullptr;
            sigemptyset(&act2.sa_mask);
            act2.sa_flags = 0;
            act2.sa_restorer = nullptr;

            if (sigaction(SIGUSR1, &act1, nullptr) < 0 ||
                sigaction(SIGUSR2, &act2, nullptr) < 0)
            {
                std::cout << "Failed to register signal handlers" << std::endl;
                return;
            }

            int pid = Kernel::getInstance()->getCurrentPid();
            std::cout << "Signal handlers registered" << std::endl;
            std::cout << "Process ID: " << pid << std::endl;
            std::cout << "Send SIGUSR1 or SIGUSR2 to trigger different handlers" << std::endl;
            std::cout << "Send SIGTERM to exit" << std::endl;

            // Wait for signals
            while (true)
            {
                pause();
            }
        };

        int pid = Kernel::getInstance()->createProcess(signalTest);
        std::cout << "Signal test process created with PID " << pid << std::endl;
    }
    else
    {
        std::cout << "Unknown test: " << testName << std::endl;
    }
}

void Shell::cmdExit(const std::vector<std::string> &args)
{
    std::cout << "Exiting..." << std::endl;
    stop();
}