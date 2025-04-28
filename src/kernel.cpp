#include "../include/kernel.h"
#include <iostream>
#include <thread>

// Initialize static members
Kernel *Kernel::instance = nullptr;
std::mutex Kernel::mtx;

Kernel::Kernel() : nextPid(1)
{
    // Initialize kernel
}

Kernel *Kernel::getInstance()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (instance == nullptr)
    {
        instance = new Kernel();
    }
    return instance;
}

int Kernel::createProcess(std::function<void()> entryPoint)
{
    std::lock_guard<std::mutex> lock(processMtx);

    // Create a new process with the next available PID
    int pid = nextPid++;
    Process *process = new Process(pid);
    processes[pid] = process;

    // Start the process thread
    process->start(entryPoint);

    return pid;
}

Process *Kernel::getProcess(int pid)
{
    std::lock_guard<std::mutex> lock(processMtx);

    auto it = processes.find(pid);
    if (it != processes.end())
    {
        return it->second;
    }
    return nullptr;
}

void Kernel::terminateProcess(int pid)
{
    std::lock_guard<std::mutex> lock(processMtx);

    // Find process
    auto it = processes.find(pid);
    if (it != processes.end())
    {
        Process *process = it->second;

        // Signal process to terminate
        process->interrupt();
        process->setState(ZOMBIE);

        // Wait for process to complete
        process->waitForCompletion();

        // Clean up
        delete process;
        processes.erase(it);
    }
}

int Kernel::sendSignal(int pid, int signum)
{
    Process *process = getProcess(pid);
    if (process == nullptr)
    {
        return -1; // Process not found
    }

    return process->sendSignal(signum);
}

// Syscall implementations
int Kernel::sys_kill(int pid, int signum)
{
    return sendSignal(pid, signum);
}

int Kernel::sys_sigaction(int signum, const SigAction *act, SigAction *oldact)
{
    int currentPid = getCurrentPid();
    Process *process = getProcess(currentPid);
    if (process == nullptr)
    {
        return -1; // Process not found
    }

    return process->registerSignalAction(signum, act, oldact);
}

int Kernel::sys_pause()
{
    // Get current process
    int currentPid = getCurrentPid();
    Process *process = getProcess(currentPid);
    if (process == nullptr)
    {
        return -1; // Process not found
    }

    // Sleep until a signal is delivered
    while (!process->isInterrupted())
    {
        process->processSignals();
        if (process->getState() != RUNNING)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

int Kernel::sys_sigemptyset(SigSet *set)
{
    if (set == nullptr)
    {
        return -1;
    }

    for (size_t i = 0; i < set->size(); i++)
    {
        (*set)[i] = false;
    }

    return 0;
}

int Kernel::sys_sigfillset(SigSet *set)
{
    if (set == nullptr)
    {
        return -1;
    }

    for (size_t i = 0; i < set->size(); i++)
    {
        (*set)[i] = true;
    }

    return 0;
}

int Kernel::sys_sigaddset(SigSet *set, int signum)
{
    if (set == nullptr || signum <= 0 || signum > SIGMAX)
    {
        return -1;
    }

    (*set)[signum] = true;
    return 0;
}

int Kernel::sys_sigdelset(SigSet *set, int signum)
{
    if (set == nullptr || signum <= 0 || signum > SIGMAX)
    {
        return -1;
    }

    (*set)[signum] = false;
    return 0;
}

int Kernel::sys_sigismember(const SigSet *set, int signum)
{
    if (set == nullptr || signum <= 0 || signum > SIGMAX)
    {
        return -1;
    }

    return (*set)[signum] ? 1 : 0;
}

int Kernel::sys_sigprocmask(int how, const SigSet *set, SigSet *oldset)
{
    // Get current process
    int currentPid = getCurrentPid();
    Process *process = getProcess(currentPid);
    if (process == nullptr)
    {
        return -1; // Process not found
    }

    // Copy the old mask if required
    if (oldset != nullptr)
    {
        // We would copy the current process's signal mask here
        // For this example, we're not implementing this fully
    }

    // Apply the new mask if required
    if (set != nullptr)
    {
        for (int signum = 1; signum <= SIGMAX; signum++)
        {
            if (signum != SIGKILL)
            { // SIGKILL can't be blocked
                if ((*set)[signum])
                {
                    process->blockSignal(signum);
                }
                else
                {
                    process->unblockSignal(signum);
                }
            }
        }
    }

    return 0;
}

int Kernel::getCurrentPid()
{
    // In a real system, this would be more complex
    // For this example, we'll assume a thread-local variable or similar
    // would track the current process ID

    // For demonstration purposes only
    static thread_local int dummyPid = 1;
    return dummyPid;
}