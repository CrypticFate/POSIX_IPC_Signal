#include "../include/process.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>

Process::Process(int id) : pid(id), state(RUNNING), interrupted(false)
{
    // Initialize signal sets
    pendingSignals.resize(SIGMAX + 1, false);
    blockedSignals.resize(SIGMAX + 1, false);
}

Process::~Process()
{
    if (thread.joinable())
    {
        thread.join();
    }
}

int Process::registerSignalAction(int signum, const SigAction *act, SigAction *oldact)
{
    // Validate signal number
    if (signum <= 0 || signum > SIGMAX)
    {
        return -1; // Invalid signal number
    }

    // Special case: SIGKILL cannot be caught or ignored
    if (signum == SIGKILL)
    {
        return -1;
    }

    // If oldact is not null, copy the old action
    if (oldact != nullptr)
    {
        auto it = sigActions.find(signum);
        if (it != sigActions.end())
        {
            *oldact = it->second;
        }
        else
        {
            // Default action: set handler to nullptr
            oldact->sa_handler = nullptr;
            oldact->sa_sigaction = nullptr;
            oldact->sa_mask.resize(SIGMAX + 1, false);
            oldact->sa_flags = 0;
            oldact->sa_restorer = nullptr;
        }
    }

    // If act is not null, set the new action
    if (act != nullptr)
    {
        sigActions[signum] = *act;
    }

    return 0; // Success
}

int Process::sendSignal(int signum)
{
    // Validate signal number
    if (signum <= 0 || signum > SIGMAX)
    {
        return -1; // Invalid signal number
    }

    // If signal is not blocked, mark it as pending
    if (!isSignalBlocked(signum))
    {
        pendingSignals[signum] = true;

        // Special handling for certain signals
        if (signum == SIGKILL)
        {
            // SIGKILL always terminates the process
            interrupt();
            setState(ZOMBIE);
            return 0;
        }
    }

    return 0; // Success
}

bool Process::isSignalBlocked(int signum) const
{
    if (signum <= 0 || signum > SIGMAX)
    {
        return false;
    }
    return blockedSignals[signum];
}

void Process::blockSignal(int signum)
{
    if (signum > 0 && signum <= SIGMAX && signum != SIGKILL)
    {
        blockedSignals[signum] = true;
    }
}

void Process::unblockSignal(int signum)
{
    if (signum > 0 && signum <= SIGMAX)
    {
        blockedSignals[signum] = false;
    }
}

void Process::processSignals()
{
    // Process all pending signals
    for (int signum = 1; signum <= SIGMAX; signum++)
    {
        if (pendingSignals[signum] && !blockedSignals[signum])
        {
            // Mark signal as handled
            pendingSignals[signum] = false;

            // Check if we have a handler registered
            auto it = sigActions.find(signum);
            if (it != sigActions.end() && it->second.sa_handler != nullptr)
            {
                // Call the handler with the signal number
                it->second.sa_handler(signum);
            }
            else if (it != sigActions.end() && it->second.sa_sigaction != nullptr)
            {
                // Prepare signal info
                SigInfo info;
                info.si_signo = signum;
                info.si_code = 0; // Default code
                info.si_pid = 0;  // Not tracking sender PID
                info.si_addr = nullptr;

                // Call the extended handler
                it->second.sa_sigaction(signum, &info, nullptr);
            }
            else
            {
                // Default action depends on the signal
                if (signum == SIGTERM || signum == SIGINT)
                {
                    // Default action: terminate
                    interrupt();
                    setState(ZOMBIE);
                    break;
                }
                // Other signals may have different default actions
            }
        }
    }
}

void Process::start(std::function<void()> func)
{
    // Create a wrapper function that handles signals
    thread = std::thread([this, func]()
                         {
        while (!isInterrupted()) {
            // Check for signals before executing
            processSignals();
            
            // If we're still running, execute the function
            if (!isInterrupted() && getState() == RUNNING) {
                func();
                // Usually, we would exit after func() completes
                break;
            }
            
            // Yield to other threads
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });
}

void Process::waitForCompletion()
{
    if (thread.joinable())
    {
        thread.join();
    }
}