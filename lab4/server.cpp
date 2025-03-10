#include"info.hpp"
#include<windows.h>
#include<iostream>
#include <sstream>
#include<string>
#include <vector>
#include<stack>
#include<queue>

class CalcServer {

    HANDLE hPipe;
    char buffer[PIPE_BUFFER_SIZE];
    std::string response = "";


    void init_pipe() {
        std::cout<<"Server log: Pipe Initialization..."<<std::endl;
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
            std::cerr << "Server log: CreateNamedPipe failed" << std::endl;
        }else {
            std::cout<<"Server log: Pipe successfully created"<<std::endl;
        }
    }

    void connect_pipe(){
        std::cout<<"Server log: Pipe Connecting..."<<std::endl;
        if (!(ConnectNamedPipe(hPipe, nullptr))) {
            std::cerr << "Server log: ConnectNamedPipe failed" << std::endl;
            CloseHandle(hPipe);
        }else {
            std::cout<<"Server log: Pipe successfully connected"<<std::endl;
        }
    }

    static bool check_correct_number(std::string number) {
        int dot_count = 0;
        for (int i = number.size() - 1; i >= 0; i--) {
            if (number[i] == '.') {
                dot_count++;
                if (dot_count > 1) {
                    return false;
                }
            }else if (!isdigit(number[i])) {
                return false;
            }
        }
        return true;
    }

    bool add_to_response(std::string str) {
        if (response == "") {
            if (check_correct_number(str)) {
                response = str;
                return true;
            }
            return false;
        }
        if (str[0] == '+' || str[0] == '-' || str[0] == '*' || str[0] == '/') {
            const char type = str[0];
            str.erase(0,1);
            if (check_correct_number(str)) {
                response = response + type + str;
                return true;
            }
            return false;
        }
        return false;
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

    int precedence(const char op) {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
    }

    double calculate(std::string& expr) {
        std::stack<char> operators;
        std::queue<std::string> output;
        std::string number;

        for (size_t i = 0; i < expr.length(); i++) {
            char ch = expr[i];

            if (isdigit(ch) || ch == '.') {
                number += ch;
            } else {
                if (!number.empty()) {
                    output.push(number);
                    number.clear();
                }

                if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                    while (!operators.empty() && precedence(operators.top()) >= precedence(ch)) {
                        output.push(std::string(1, operators.top()));
                        operators.pop();
                    }
                    operators.push(ch);
                }
            }
        }

        if (!number.empty()) output.push(number);

        while (!operators.empty()) {
            output.push(std::string(1, operators.top()));
            operators.pop();
        }

        std::stack<double> values;
        while (!output.empty()) {
            std::string token = output.front();
            output.pop();

            if (isdigit(token[0])) {
                values.push(stod(token));
            } else {
                const double b = values.top(); values.pop();
                const double a = values.top(); values.pop();

                if (token == "+") values.push(a + b);
                if (token == "-") values.push(a - b);
                if (token == "*") values.push(a * b);
                if (token == "/") values.push(a / b);
            }
        }

        return values.top();
    }

    void send_message(const std::string &message) {
        WriteFile(
            hPipe,
            message.c_str(),
            message.size()+1,
            nullptr,
            nullptr);
    }

    void response_request() {
        while (true) {
            std::cout<<"Server log: Wait request..."<<std::endl;
            memset(buffer, 0, PIPE_BUFFER_SIZE);

            if (!(ReadFile(
                hPipe,
                buffer,
                PIPE_BUFFER_SIZE,
                nullptr,
                nullptr))) {
                std::cerr << "Server log: Disconnect from user" << std::endl;
                break;
                }

            std::string request = std::string(buffer);
            std::string response_message;
            switch (check_command(request)) {
                case commands::COMMON_MESSAGE:
                    if (add_to_response(request)) {
                        response_message = "Successfully add: "+request + "\nCurrent expression: "+response;

                    }else {
                        response_message = "Incorrect number, can't add to expression: "+request + "\nCurrent expression: "+response;
                    }
                    send_message(response_message);
                    break;
                case commands::CHECK:
                    if (check_zero()) {
                        response_message = "Expression is incorrect, detected division by zero\nCurrent expression: "+response;
                        send_message(response_message);
                    }else {
                        response_message = "Expression is correct\nCurrent expression: "+response;
                        send_message(response_message);
                    }
                    break;
                case commands::CLEAR:
                    clear();
                    response_message = "Expression is clear\nCurrent expression: "+response;
                    send_message(response_message);
                    break;
                case commands::CALCULATE:
                    if (check_zero()) {
                        response_message = "Can't calculate, detected division by zero\nCurrent expression: "+response;
                        send_message(response_message);
                    }else {
                        const double result = calculate(response);
                        response_message = "Successfully calculated\nCurrent expression: "+response+"\nResult is: "+std::to_string(result);
                        send_message(response_message);
                    }
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