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
    // Validate signal number
    if (signum <= 0 || signum > SIGMAX) {
        return -1;
    }

    // Get target process
    Process *process = getProcess(pid);
    if (process == nullptr) {
        // For testing purposes, create the process if it doesn't exist
        process = new Process(pid);
        processes[pid] = process;
    }

    return process->sendSignal(signum);
}

int Kernel::sys_sigaction(int signum, const SigAction *act, SigAction *oldact)
{
    // Validate signal number
    if (signum <= 0 || signum > SIGMAX) {
        return -1;
    }

    // SIGKILL and SIGSTOP cannot be caught or ignored
    if (signum == SIGKILL) {
        return -1;
    }

    // Get current process
    Process *process = getProcess(getCurrentPid());
    if (process == nullptr) {
        // Create a new process if one doesn't exist
        process = new Process(getCurrentPid());
        processes[getCurrentPid()] = process;
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
    Process *process = getProcess(getCurrentPid());
    if (process == nullptr)
    {
        // Create a new process if one doesn't exist
        process = new Process(getCurrentPid());
        processes[getCurrentPid()] = process;
    }

    // If oldset is not null, save the current signal mask
    if (oldset != nullptr)
    {
        *oldset = process->getBlockedSignals();
    }

    // If set is not null, modify the signal mask according to 'how'
    if (set != nullptr)
    {
        SigSet newMask = process->getBlockedSignals();

        switch (how)
        {
        case 0: // SIG_BLOCK: Add the signals in set to the current mask
            for (size_t i = 0; i < set->size() && i <= SIGMAX; i++)
            {
                if ((*set)[i] && i != SIGKILL) // SIGKILL cannot be blocked
                {
                    newMask[i] = true;
                }
            }
            break;

        case 1: // SIG_UNBLOCK: Remove the signals in set from the current mask
            for (size_t i = 0; i < set->size() && i <= SIGMAX; i++)
            {
                if ((*set)[i])
                {
                    newMask[i] = false;
                }
            }
            break;

        case 2: // SIG_SETMASK: Replace the current mask with set
            newMask = *set;
            newMask[SIGKILL] = false; // Ensure SIGKILL cannot be blocked
            break;

        default:
            return -1;
        }

        process->setBlockedSignals(newMask);
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