#include "../include/process.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <mutex>

Process::Process(int id) : pid(id), state(RUNNING), interrupted(false)
{
    // Initialize signal sets
    pendingSignals.resize(SIGMAX + 1, false);
    blockedSignals.resize(SIGMAX + 1, false);
}

Process::~Process()
{
    interrupt();
    if (thread.joinable())
    {
        thread.join();
    }
}

int Process::registerSignalAction(int signum, const SigAction *act, SigAction *oldact)
{
    std::lock_guard<std::mutex> lock(signalMutex);

    std::cout << "Process " << pid << ": Registering signal handler for signal " << signum << std::endl;

    // Validate signal number
    if (signum <= 0 || signum > SIGMAX)
    {
        std::cout << "Process " << pid << ": Invalid signal number " << signum << std::endl;
        return -1;
    }

    // Special case: SIGKILL cannot be caught or ignored
    if (signum == SIGKILL)
    {
        std::cout << "Process " << pid << ": Cannot register handler for SIGKILL" << std::endl;
        return -1;
    }

    // If oldact is not null, copy the old action
    if (oldact != nullptr)
    {
        auto it = sigActions.find(signum);
        if (it != sigActions.end())
        {
            *oldact = it->second;
            std::cout << "Process " << pid << ": Saved old handler for signal " << signum << std::endl;
        }
        else
        {
            // Default action: set handler to nullptr
            oldact->sa_handler = nullptr;
            oldact->sa_sigaction = nullptr;
            oldact->sa_mask.resize(SIGMAX + 1, false);
            oldact->sa_flags = 0;
            oldact->sa_restorer = nullptr;
            std::cout << "Process " << pid << ": No previous handler for signal " << signum << std::endl;
        }
    }

    // If act is not null, set the new action
    if (act != nullptr)
    {
        // Create a copy of the action
        SigAction newAct = *act;
        
        // Ensure the signal mask is properly sized
        if (newAct.sa_mask.size() < SIGMAX + 1)
        {
            newAct.sa_mask.resize(SIGMAX + 1, false);
        }

        // Store the action
        sigActions[signum] = newAct;
        std::cout << "Process " << pid << ": Registered "
                  << (newAct.sa_handler ? "basic" : "extended")
                  << " handler for signal " << signum << std::endl;
    }
    else
    {
        // Remove the signal handler if act is null
        sigActions.erase(signum);
        std::cout << "Process " << pid << ": Removed handler for signal " << signum << std::endl;
    }

    return 0;
}

int Process::sendSignal(int signum)
{
    std::lock_guard<std::mutex> lock(signalMutex);

    std::cout << "Process " << pid << ": Received signal " << signum << std::endl;

    // Validate signal number
    if (signum <= 0 || signum > SIGMAX)
    {
        std::cout << "Process " << pid << ": Invalid signal number " << signum << std::endl;
        return -1;
    }

    // Special handling for signals that can't be blocked or ignored
    if (signum == SIGKILL)
    {
        std::cout << "Process " << pid << ": Received SIGKILL, terminating..." << std::endl;
        interrupt();
        setState(ZOMBIE);
        return 0;
    }

    // Mark signal as pending
    pendingSignals[signum] = true;
    std::cout << "Process " << pid << ": Marked signal " << signum << " as pending" << std::endl;

    return 0;
}

bool Process::isSignalBlocked(int signum) const
{
    std::lock_guard<std::mutex> lock(signalMutex);
    if (signum <= 0 || signum > SIGMAX)
    {
        return false;
    }
    return blockedSignals[signum];
}

void Process::blockSignal(int signum)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    if (signum > 0 && signum <= SIGMAX && signum != SIGKILL)
    {
        blockedSignals[signum] = true;
    }
}

void Process::unblockSignal(int signum)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    if (signum > 0 && signum <= SIGMAX)
    {
        blockedSignals[signum] = false;
    }
}

void Process::processSignals()
{
    std::lock_guard<std::mutex> lock(signalMutex);

    // Process all pending signals
    for (int signum = 1; signum <= SIGMAX; signum++)
    {
        if (pendingSignals[signum] && !blockedSignals[signum])
        {
            std::cout << "Process " << pid << ": Processing signal " << signum << std::endl;

            // Mark signal as handled
            pendingSignals[signum] = false;

            // Check if we have a handler registered
            auto it = sigActions.find(signum);
            if (it != sigActions.end())
            {
                std::cout << "Process " << pid << ": Found handler for signal " << signum << std::endl;

                // Block signals specified in sa_mask while handling
                std::vector<bool> oldMask = blockedSignals;
                for (size_t i = 0; i < it->second.sa_mask.size(); i++)
                {
                    if (it->second.sa_mask[i])
                    {
                        blockedSignals[i] = true;
                    }
                }

                // Call the appropriate handler
                if (it->second.sa_handler != nullptr)
                {
                    std::cout << "Process " << pid << ": Calling basic handler for signal " << signum << std::endl;
                    // Call the basic handler
                    it->second.sa_handler(signum);
                }
                else if (it->second.sa_sigaction != nullptr)
                {
                    std::cout << "Process " << pid << ": Calling extended handler for signal " << signum << std::endl;
                    // Prepare signal info
                    SigInfo info;
                    info.si_signo = signum;
                    info.si_code = 0;
                    info.si_pid = pid;
                    info.si_addr = nullptr;

                    // Call the extended handler
                    it->second.sa_sigaction(signum, &info, nullptr);
                }

                // Restore the old signal mask
                blockedSignals = oldMask;
                std::cout << "Process " << pid << ": Restored signal mask after handling signal " << signum << std::endl;
            }
            else
            {
                std::cout << "Process " << pid << ": No handler found for signal " << signum << std::endl;

                // Default action depends on the signal
                switch (signum)
                {
                case SIGTERM:
                case SIGINT:
                case SIGQUIT:
                case SIGILL:
                case SIGABRT:
                    std::cout << "Process " << pid << ": Default action for signal " << signum << ": terminate" << std::endl;
                    interrupt();
                    setState(ZOMBIE);
                    break;
                case SIGHUP:
                case SIGTRAP:
                    std::cout << "Process " << pid << ": Default action for signal " << signum << ": stop" << std::endl;
                    setState(STOPPED);
                    break;
                default:
                    std::cout << "Process " << pid << ": No default action for signal " << signum << std::endl;
                    break;
                }
            }
        }
    }
}

void Process::start(std::function<void()> func)
{
    // Create a wrapper function that handles signals
    thread = std::thread([this, func]()
    {
        while (!isInterrupted())
        {
            // Process any pending signals
            processSignals();

            // If we're still running, execute the function
            if (!isInterrupted() && getState() == RUNNING)
            {
                try
                {
                    func();
                    // After executing the function, process any pending signals
                    processSignals();
                }
                catch (...)
                {
                    // Handle any exceptions from the function
                    setState(ZOMBIE);
                    break;
                }
            }

            // Small delay to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
}

void Process::waitForCompletion()
{
    if (thread.joinable())
    {
        thread.join();
    }
}