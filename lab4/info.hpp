#pragma once
#include <string>

#define PIPE_NAME "\\\\.\\pipe\\CalcPipe"
#define PIPE_BUFFER_SIZE 10000


enum commands {
    CALCULATE,
    CLEAR,
    EXIT,
    CHECK,
    COMMON_MESSAGE
};

inline commands check_command(const std::string& command) {
    if (command == "CALCULATE" || command == "=")
        return CALCULATE;
    if (command == "CLEAR")
        return CLEAR;
    if (command == "EXIT")
        return EXIT;
    if (command == "CHECK")
        return CHECK;
    return COMMON_MESSAGE;
}

