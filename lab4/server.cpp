#include"info.hpp"
#include<windows.h>
#include<iostream>
#include<string>
#include <vector>

class CalcServer {
    HANDLE hPipe;
    char request_response[PIPE_BUFFER_SIZE];
    std::string response = "";
    void init_pipe() {
        std::cout<<"Pipe Initialization..."<<std::endl;
        hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            PIPE_BUFFER_SIZE,
            PIPE_BUFFER_SIZE,
            100,
            nullptr);

        if (hPipe == INVALID_HANDLE_VALUE) {
            std::cerr << "CreateNamedPipe() failed" << std::endl;
        }else {
            std::cout<<"Pipe successfully created"<<std::endl;
        }
    }

    void connect_pipe(){
        std::cout<<"Pipe Connecting..."<<std::endl;
        if (!(ConnectNamedPipe(hPipe, nullptr))) {
            std::cerr << "ConnectNamedPipe() failed" << std::endl;
            CloseHandle(hPipe);
        }else {
            std::cout<<"Pipe successfully connected"<<std::endl;
        }
    }

    void add_to_response(std::string str) {
        if (str[0] == '-' || str[0] == '+' || str[0] == '/' || str[0] == '*') {
            char temp = str[0];
            str.erase(0, 1);
            try {
                std::stoi(str, nullptr, 10);
                std::cout << "Successfully added" << std::endl;
                response = response + temp +str;

            }catch (std::out_of_range) {
                std::cout<<"Invalid input"<<std::endl;
            }catch (std::invalid_argument) {
                std::cout<<"Invalid input"<<std::endl;
            }
        }else {
            std::cout<<"Invalid input"<<std::endl;
        }
    }

    bool check_zero() {
        for (int i = 0; i < response.length()-1; i++) {
            if (response[i] == '/' && response[i+1] == '0') {
                return true;
            }
        }
        return false;
    }

    void clear() {
        response = "";
    }

    double calculate() {
        return 0;
    }

    void send_message(const std::string &message) {
        WriteFile(
            hPipe,
            message.c_str(),
            PIPE_BUFFER_SIZE,
            nullptr,
            nullptr);
    }

    void response_request() {
        while (true) {
            std::cout<<"Pipe Request Response..."<<std::endl;
            memset(request_response, 0, PIPE_BUFFER_SIZE);

            if (!(ReadFile(
                hPipe,
                request_response,
                PIPE_BUFFER_SIZE,
                nullptr,
                nullptr))) {
                std::cerr << "ReadFile() failed" << std::endl;
                break;
                }

            std::string request = std::string(request_response);

            switch (check_command(request)) {
                case commands::COMMON_MESSAGE:
                    add_to_response(request);
                    send_message(response);
                    break;
                case commands::CHECK:
                    if (check_zero()) {
                        send_message("1");
                    }else {
                        send_message("0");
                    }
                    break;
                case commands::CLEAR:
                    clear();
                    send_message("Clear");
                    break;
                case commands::CALCULATE:
                    double result = calculate();
                    send_message(std::to_string(result));
                    break;
            }
        }
        CloseHandle(hPipe);
    }

    public:
    CalcServer() = default;
    ~CalcServer() = default;

    void run() {
        init_pipe();
        connect_pipe();
        response_request();
    }
};

int main() {
    CalcServer calc_server;
    calc_server.run();
}