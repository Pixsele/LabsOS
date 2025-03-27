#include <iostream>
#include <ostream>
#include <string>

#include"info.hpp"
#include<windows.h>

class Client {
    HANDLE hPipe{};
    char response_buffer[PIPE_BUFFER_SIZE];

    void connect() {
        if (!(WaitNamedPipeA(PIPE_NAME, 1000))) {
            std::cerr << "Pipe not available" << std::endl;
        }
        hPipe = CreateFileA(PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);
        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateFileA failed with error: " << GetLastError() << std::endl;
        }else {
            std::cout << "Connected" << std::endl;
        }
    }

    void process() {
        while (true) {
            std::string message;
            getline(std::cin, message);

            bool success_write = WriteFile(
                hPipe,
                message.c_str(),
                message.size()+1,
                nullptr,
                nullptr);

            if (!success_write) {
                std::cerr << "Server disconnected" << std::endl;
                CloseHandle(hPipe);
                break;
            }

            memset(response_buffer, 0, PIPE_BUFFER_SIZE);
            bool success_read = ReadFile(
                hPipe,
                response_buffer,
                PIPE_BUFFER_SIZE,
                nullptr,
                nullptr);

            if (!success_read) {
                std::cerr << "Server disconnected" << std::endl;
                CloseHandle(hPipe);
                break;
            }

            if (response_buffer[0] == 'q') {
                std::cout << "Disconnect" << std::endl;
                CloseHandle(hPipe);
                break;
            }

            std::cout << response_buffer << std::endl;
        }
    }


    public:
    Client() = default;
    ~Client() = default;

    void run() {
        connect();
        process();
    }
};

int main() {
    Client client;
    client.run();
}



