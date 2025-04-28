#include "../include/shell.h"
#include "../include/kernel.h"
#include <iostream>
#include <signal.h>

int main(int argc, char *argv[])
{
    // Initialize our kernel singleton
    Kernel *kernel = Kernel::getInstance();

    // Create our shell
    Shell shell;

    // Welcome message
    std::cout << "==================================" << std::endl;
    std::cout << "  POSIX-like Signal OS Simulator  " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;

    // Run the shell
    shell.run();

    std::cout << "Shutting down..." << std::endl;

    return 0;
}