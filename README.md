# POSIX-like Signal OS Project

This project implements a simulation of POSIX signal handling functionality in a custom OS environment. It allows users to register custom signal handlers and demonstrates inter-process communication through signals.

## Features

- Custom implementation of `sigaction()` for registering signal handlers
- Implementation of `kill()` for sending signals between processes
- Support for signal blocking and unblocking
- Shell interface for sending signals to processes
- Test programs demonstrating signal handling

## Signal Handlers

The project supports two types of signal handlers:

1. Simple handlers: `void (*handler)(int)` - Receives the signal number
2. Extended handlers: `void (*sa_sigaction)(int, siginfo_t *, void *)` - Receives signal info

## Project Structure

- `include/` - Header files
- `src/` - Implementation files
- `test/` - Test programs

## Building the Project

To build the project, simply run:

```
make
```

This will build the main program (`signal_os`) and test programs.

## Running the Project

To run the main program:

```
make run
```

This will start the shell interface where you can enter commands.

## Running Tests

To run all tests:

```
make run_tests
```

To run a specific test (e.g., `test_trap`):

```
make run_test_trap
```

## Shell Commands

The shell supports the following commands:

- `help` - Display help information
- `kill [-NUM] PID` - Send signal NUM to process PID
- `ps` - List all processes
- `run TEST` - Run a test program (trap, kill, signals)
- `exit`, `quit` - Exit the shell

## Test Programs

1. `test_trap` - Demonstrates signal trapping functionality
2. `test_kill` - Demonstrates kill functionality
3. `test_signals` - Comprehensive test of all signal behaviors

## Run all the test cases:
```
sudo make clean && make all && ./test/test_signals
```


## Signal API

Key functions implemented in this project:

- `int sigaction(int signum, const SigAction *act, SigAction *oldact)`
- `int kill(int pid, int sig)`
- `int pause()`
- `int sigemptyset(SigSet *set)`
- `int sigfillset(SigSet *set)`
- `int sigaddset(SigSet *set, int signum)`
- `int sigdelset(SigSet *set, int signum)`
- `int sigismember(const SigSet *set, int signum)`
- `int sigprocmask(int how, const SigSet *set, SigSet *oldset)`
- `void (*signal(int signo, void (*func)(int)))(int)`

## Requirements

- C++17 compiler
- POSIX-compatible system
- pthread support
