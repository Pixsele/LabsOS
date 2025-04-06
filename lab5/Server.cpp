#include "info.hpp"

#include <WinSock2.h>
#include <iostream>
#include <list>
#include <queue>
#include <stack>
#include <thread>
#include <vector>

SOCKET sock;

std::vector<SOCKET> run_sockets;
HANDLE m_client_mutex;
HANDLE m_semaphore;
std::string current_expression = "";

void launch_client() {
    const auto run_client = R"(start C:\Prog\LabsOS\lab5\Client.exe)";
    system(run_client);
}

bool check_correct_number(std::string number) {
    if (number[0] == '-') {
        number.erase(0, 1);
    }
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
    if (current_expression.empty()) {
        if (check_correct_number(str)) {
            current_expression = str;
            return true;
        }
        return false;
    }
    if (str[0] == '+' || str[0] == '-' || str[0] == '*' || str[0] == '/') {
        const char type = str[0];
        str.erase(0,1);
        if (str.length() == 0) {
            return false;
        }
        if (check_correct_number(str)) {
            current_expression = current_expression + type + str;
            return true;
        }
        return false;
    }
    return false;
}

bool check_zero() {
    for (int i = 0; i < current_expression.length() - 1; i++) {
        if (current_expression[i] == '/' && current_expression[i + 1] == '0') {
            return false;
        }
    }
    return true;
}

int precedence(const char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

std::string calculate(std::string &expr) {
    std::stack<char> operators;
    std::queue<std::string> output;
    std::string number;
    bool lastWasOperator = true;

    for (size_t i = 0; i < expr.length(); i++) {
        char ch = expr[i];

        if (isdigit(ch) || ch == '.') {
            number += ch;
            lastWasOperator = false;
        } else if (ch == '-' && lastWasOperator) {
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
                lastWasOperator = true;
            } else {
                lastWasOperator = false;
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

        if (isdigit(token[0]) || (token.length() > 1 && token[0] == '-')) {
            values.push(stod(token));
        } else {
            const double b = values.top();
            values.pop();
            const double a = values.top();
            values.pop();

            if (token == "+") values.push(a + b);
            if (token == "-") values.push(a - b);
            if (token == "*") values.push(a * b);
            if (token == "/") values.push(a / b);
        }
    }
    return std::to_string(values.top());
}

void process_client(const int i) {
    SOCKET current_user = run_sockets[i];

    WaitForSingleObject(m_semaphore, INFINITE);

    while (true) {
        std::string message;
        char buffer[1024];

        memset(buffer, 0, sizeof(buffer));

        if (recv(current_user, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
            std::cerr  << "Failed to receive message from client " << i + 1 << std::endl;
            closesocket(current_user);
            ReleaseMutex(m_semaphore);
        }

        message = std::string(buffer);

        if (message == "CLEAR") {
            WaitForSingleObject(m_client_mutex, INFINITE);
            current_expression.clear();
            std::string message_to_send = "Exprassion is clear";
            if (send(current_user, message_to_send.c_str(), message_to_send.length() + 1, 0) == SOCKET_ERROR) {
                std::cerr  << "Failed to send message to client " << i + 1 << std::endl;
            }
            ReleaseMutex(m_client_mutex);
        }else if (message == "CALCULATE" || message == "=") {
            WaitForSingleObject(m_client_mutex, INFINITE);
            if (check_zero()) {
                std::string message_to_send = "Current expression: " + current_expression + "\nCalculate result: " + calculate(current_expression);
                if (send(current_user, message_to_send.c_str(), message_to_send.length() + 1, 0) == SOCKET_ERROR) {
                    std::cerr  << "Failed to send message to client " << i + 1 << std::endl;
                }
            }else {
                std::string message_to_send = "Exprassion is incorrect";
                if (send(current_user, message_to_send.c_str(), message_to_send.length() + 1, 0) == SOCKET_ERROR) {
                    std::cerr  << "Failed to send message to client " << i + 1 << std::endl;
                }
            }
            ReleaseMutex(m_client_mutex);
        }else {
            WaitForSingleObject(m_client_mutex, INFINITE);
            if (add_to_response(message)) {
                std::string message_to_send = "Successfully added response: " + message + "\nCurrent expression: " + current_expression;
                if (send(current_user, message_to_send.c_str(), message_to_send.size()+1, 0) == SOCKET_ERROR) {
                    std::cerr << "Failed send response" << std::endl;
                }
            }else {
                std::string message_to_send = "Incorrect math expression: " + message + "\nCurrent expression: " + current_expression;
                if (send(current_user, message_to_send.c_str(), message_to_send.size()+1, 0) == SOCKET_ERROR) {
                    std::cerr << "Failed send response" << std::endl;
                }
            }
            ReleaseMutex(m_client_mutex);
        }
    }
}

void init() {
    m_client_mutex = CreateMutexA(nullptr, false, "client_mutex");

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(info::IP);
    addr.sin_port = htons(info::SERVER_PORT);
    addr.sin_family = AF_INET;

    sock = socket(AF_INET, SOCK_STREAM, NULL);

    if (bind(sock, (SOCKADDR *) &addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind" << std::endl;
        closesocket(sock);
        WSACleanup();
        exit(1);
    };

    std::cout << "Server Log: Bind done" << std::endl;

    if (listen(sock, info::MAX_QUEUE_SIZE) == SOCKET_ERROR) {
        std::cerr << "Failed to listen" << std::endl;
        closesocket(sock);
        WSACleanup();
        exit(1);
    }

    std::cout << "Server Log: Listen start" << std::endl;

    SOCKET accept_socket;

    int count_of_clients;
    std::cin >> count_of_clients;

    m_semaphore = CreateSemaphoreA(nullptr, count_of_clients, count_of_clients, "semaphore");

    for (int i = 0; i < count_of_clients; i++) {
        std::cout << "Try Client " << i << " to connect" <<std::endl;
        launch_client();
        accept_socket = accept(sock, (SOCKADDR*)&addr, NULL);

        if (!accept_socket) {
            std::cerr << "Failed to accept client: " << i << std::endl;
            if (closesocket(sock) == SOCKET_ERROR) {
                std::cerr << "Failed to terminate connection: " << WSAGetLastError() << std::endl;
            }
            continue;
        }

        run_sockets.push_back(accept_socket);
        std::cout << "Client " << i << " connected" << std::endl;
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < run_sockets.size(); i++) {
        if (run_sockets[i] != INVALID_SOCKET) {
            threads.emplace_back(process_client, i);
        }
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "WSAStartup successful" << std::endl;
    init();
}
