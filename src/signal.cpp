#include "../include/signal.h"
#include "../include/kernel.h"
#include <iostream>

// System call wrappers for signal-related functionality

int sigaction(int signum, const SigAction *act, SigAction *oldact)
{
    // Call into the kernel to handle the sigaction syscall
    return Kernel::getInstance()->sys_sigaction(signum, act, oldact);
}

int kill(int pid, int sig)
{
    // Call into the kernel to handle the kill syscall
    return Kernel::getInstance()->sys_kill(pid, sig);
}

int pause()
{
    // Call into the kernel to handle the pause syscall
    return Kernel::getInstance()->sys_pause();
}

int sigemptyset(SigSet *set)
{
    // Call into the kernel to handle the sigemptyset syscall
    return Kernel::getInstance()->sys_sigemptyset(set);
}

int sigfillset(SigSet *set)
{
    // Call into the kernel to handle the sigfillset syscall
    return Kernel::getInstance()->sys_sigfillset(set);
}

int sigaddset(SigSet *set, int signum)
{
    // Call into the kernel to handle the sigaddset syscall
    return Kernel::getInstance()->sys_sigaddset(set, signum);
}

int sigdelset(SigSet *set, int signum)
{
    // Call into the kernel to handle the sigdelset syscall
    return Kernel::getInstance()->sys_sigdelset(set, signum);
}

int sigismember(const SigSet *set, int signum)
{
    // Call into the kernel to handle the sigismember syscall
    return Kernel::getInstance()->sys_sigismember(set, signum);
}

int sigprocmask(int how, const SigSet *set, SigSet *oldset)
{
    // Call into the kernel to handle the sigprocmask syscall
    return Kernel::getInstance()->sys_sigprocmask(how, set, oldset);
}

// Helper function to match POSIX signal function prototype
void (*signal(int signo, void (*func)(int)))(int)
{
    SigAction act, oldact;

    // Initialize action structure
    act.sa_handler = func;
    act.sa_sigaction = nullptr;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_restorer = nullptr;

    // Register the signal handler
    if (sigaction(signo, &act, &oldact) < 0)
    {
        // return SIG_ERR;
    }

    // Return the old handler
    return oldact.sa_handler;
}