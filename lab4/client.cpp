#include <iostream>
#include <ostream>
#include <string>

#include"info.hpp"
#include<windows.h>

class Client {
    HANDLE hPipe{};
    char response_buffer[PIPE_BUFFER_SIZE];

    void connect() {
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

            WriteFile(
                hPipe,
                message.c_str(),
                message.size()+1,
                nullptr,
                nullptr);

            memset(response_buffer, 0, PIPE_BUFFER_SIZE);
            ReadFile(
                hPipe,
                response_buffer,
                PIPE_BUFFER_SIZE,
                nullptr,
                nullptr);

            if (response_buffer[0] == 'q') {
                std::cout << "Disconnect" << std::endl;
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



