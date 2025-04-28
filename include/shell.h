#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <vector>
#include <functional>

class Shell
{
private:
    bool running;

    // Command handlers
    void cmdHelp(const std::vector<std::string> &args);
    void cmdKill(const std::vector<std::string> &args);
    void cmdPs(const std::vector<std::string> &args);
    void cmdRun(const std::vector<std::string> &args);
    void cmdExit(const std::vector<std::string> &args);

    // Command parsing
    std::vector<std::string> parseCommand(const std::string &cmd);

public:
    Shell();
    ~Shell();

    void run();
    void stop();
};

#endif // SHELL_H