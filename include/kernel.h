#ifndef KERNEL_H
#define KERNEL_H

#include <map>
#include <mutex>
#include <vector>
#include "process.h"
#include "signal.h"

class Kernel
{
private:
    static Kernel *instance;
    static std::mutex mtx;

    std::map<int, Process *> processes; // Map of process ID to Process object
    int nextPid;                        // Next available PID
    std::mutex processMtx;              // Mutex for process operations

    // Private constructor for singleton
    Kernel();

public:
    // Singleton pattern
    static Kernel *getInstance();

    // Process management
    int createProcess(std::function<void()> entryPoint);
    Process *getProcess(int pid);
    void terminateProcess(int pid);

    // Signal management
    int sendSignal(int pid, int signum);

    // Syscall implementations
    int sys_kill(int pid, int signum);
    int sys_sigaction(int signum, const SigAction *act, SigAction *oldact);
    int sys_pause();
    int sys_sigemptyset(SigSet *set);
    int sys_sigfillset(SigSet *set);
    int sys_sigaddset(SigSet *set, int signum);
    int sys_sigdelset(SigSet *set, int signum);
    int sys_sigismember(const SigSet *set, int signum);
    int sys_sigprocmask(int how, const SigSet *set, SigSet *oldset);

    // Utility functions
    int getCurrentPid();
};

#endif // KERNEL_H