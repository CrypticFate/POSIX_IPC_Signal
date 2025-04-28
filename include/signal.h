// Signal definitions and API declarations
#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <functional>
#include <vector>

// Signal numbers (similar to POSIX signals)
enum SignalNum
{
    SIGHUP = 1,   // Hangup
    SIGINT = 2,   // Interrupt from keyboard (Ctrl+C)
    SIGQUIT = 3,  // Quit from keyboard
    SIGILL = 4,   // Illegal instruction
    SIGTRAP = 5,  // Trace/breakpoint trap
    SIGABRT = 6,  // Abort signal
    SIGKILL = 9,  // Kill signal (cannot be caught or ignored)
    SIGUSR1 = 10, // User-defined signal 1
    SIGUSR2 = 11, // User-defined signal 2
    SIGALRM = 14, // Alarm clock
    SIGTERM = 15, // Termination signal
    // Add more signals as needed
    SIGMAX = 32 // Maximum signal number
};

// Signal information structure (similar to siginfo_t)
struct SigInfo
{
    int si_signo;  // Signal number
    int si_code;   // Signal code
    int si_pid;    // Sending process ID
    void *si_addr; // Memory location that caused fault
    // Add more fields as needed
};

// Signal action structure (similar to struct sigaction)
struct SigAction
{
    void (*sa_handler)(int);                      // Signal handler function
    void (*sa_sigaction)(int, SigInfo *, void *); // Extended signal handler
    std::vector<bool> sa_mask;                    // Signal mask during handler execution
    int sa_flags;                                 // Signal flags
    void (*sa_restorer)(void);                    // Signal restorer function
};

// Signal set type (similar to sigset_t)
typedef std::vector<bool> SigSet;

// Signal API functions
int sigaction(int signum, const SigAction *act, SigAction *oldact);
int kill(int pid, int sig);
int pause();
int sigemptyset(SigSet *set);
int sigfillset(SigSet *set);
int sigaddset(SigSet *set, int signum);
int sigdelset(SigSet *set, int signum);
int sigismember(const SigSet *set, int signum);
int sigprocmask(int how, const SigSet *set, SigSet *oldset);

#endif // SIGNAL_H