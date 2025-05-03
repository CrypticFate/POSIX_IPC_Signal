#ifndef PROCESS_H
#define PROCESS_H

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include "signal.h"

// Process States
enum ProcessState
{
    RUNNING,
    WAITING,
    STOPPED,
    ZOMBIE
};

// Process Control Block structure
class Process
{
private:
    int pid;                             // Process ID
    ProcessState state;                  // Current state
    std::thread thread;                  // Actual thread
    std::map<int, SigAction> sigActions; // Signal action mappings
    SigSet pendingSignals;               // Pending signals
    SigSet blockedSignals;               // Blocked signals
    bool interrupted;                    // Flag for interruption
    mutable std::mutex signalMutex;       // Mutex for signal operations

public:
    Process(int id);
    ~Process();

    // Getters/Setters
    int getPid() const { return pid; }
    ProcessState getState() const { return state; }
    void setState(ProcessState newState) { state = newState; }

    // Signal handling
    int registerSignalAction(int signum, const SigAction *act, SigAction *oldact);
    int sendSignal(int signum);
    bool isSignalBlocked(int signum) const;
    void blockSignal(int signum);
    void unblockSignal(int signum);
    void processSignals();

    // Signal mask management
    const SigSet& getBlockedSignals() const { 
        std::lock_guard<std::mutex> lock(signalMutex);
        return blockedSignals; 
    }
    void setBlockedSignals(const SigSet& mask) { 
        std::lock_guard<std::mutex> lock(signalMutex);
        blockedSignals = mask;
    }

    // Thread management
    void start(std::function<void()> func);
    void waitForCompletion();
    bool isInterrupted() const { return interrupted; }
    void interrupt() { interrupted = true; }
};

#endif // PROCESS_H