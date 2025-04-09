#include "info.hpp"

#include <WinSock2.h>
#include <iostream>
#include <list>
#include <queue>
#include <sstream>
#include <stack>
#include <thread>
#include <vector>

SOCKET sock;

std::vector<SOCKET> run_sockets;
HANDLE m_client_semaphore;

void launch_client() {
    const auto run_client = R"(start C:\Progs\LabsOS\lab5\lab5.1\Client.exe)";
    system(run_client);
}
int check_correct_count_of_clients() {
    int input = 0;
    while (!(std::cin >> input) || std::cin.peek() != '\n' || input > 2 || input < 1) {
        std::cin.clear();
        while (std::cin.get() != '\n'){}
        std::cout << "Please enter a number between 1 and 2: ";
    }
    return input;
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

bool check_correct_math_expression(std::vector<std::string>& tokens,std::string current_expression) {

    std::vector<std::string> words;
    std::stringstream ss(current_expression);
    std::string word;

    while (ss >> word) {
        words.push_back(word);
    }

    if (words.size() < 3) {
        return false;
    }

    bool last_was_digit = true;
    for (int i = 0; i < words.size(); i++) {
        const std::string& word = words[i];
        if (i == 0) {
            if (!check_correct_number(word)) {
                return false;
            }
        }else {
            if (last_was_digit) {
                if (word == "-" || word == "+" || word == "*" || word == "/") {
                    last_was_digit = false;
                }
                else {
                    return false;
                }
            }else {
                if (check_correct_number(word)) {
                    last_was_digit = true;
                }else {
                    return false;
                }
            }
        }
    }

    if (word[word.size() - 1] == '+' || word[word.size() - 1] == '-' || word[word.size() - 1] == '*' || word[word.size() - 1] == '/') {
        return false;
    }
    tokens = words;
    return true;
}

bool check_zero(const std::vector<std::string> &tokens) {
    for (int i = 0; i < tokens.size() - 1; i++) {
        if (tokens[i] == "/" && stod(tokens[i + 1]) == 0) {
            return false;
        }
    }
    return true;
}

int precedence(const std::string& op) {
    if (op == "+" || op == "-") return 1;
    if (op == "*" || op == "/") return 2;
    return 0;
}

std::string calculate(std::vector<std::string> &expr) {
    std::stack<std::string> operators;
    std::queue<std::string> output;
    std::stack<double> values;

    for (size_t i = 0; i < expr.size(); i++) {
        const std::string &token = expr[i];
        if (isdigit(token[0]) || (token[0] == '-' && token.length() > 1 && isdigit(token[1]))) {
            values.push(std::stod(token));
        } else if (token == "+" || token == "-" || token == "*" || token == "/") {
            while (!operators.empty() && precedence(operators.top()) >= precedence(token)) {
                std::string op = operators.top();
                operators.pop();

                double b = values.top(); values.pop();
                double a = values.top(); values.pop();

                if (op == "+") values.push(a + b);
                if (op == "-") values.push(a - b);
                if (op == "*") values.push(a * b);
                if (op == "/") values.push(a / b);
            }

            operators.push(token);
        }
    }

    while (!operators.empty()) {
        std::string op = operators.top();
        operators.pop();

        double b = values.top(); values.pop();
        double a = values.top(); values.pop();

        if (op == "+") values.push(a + b);
        if (op == "-") values.push(a - b);
        if (op == "*") values.push(a * b);
        if (op == "/") values.push(a / b);
    }
    return std::to_string(values.top());
}

void process_client(const int i) {

    SOCKET current_user = run_sockets[i];
    while (true) {
        std::string message;
        char buffer[1024];

        memset(buffer, 0, sizeof(buffer));

        if (recv(current_user, buffer, sizeof(buffer) - 1, 0) == SOCKET_ERROR) {
            std::cerr  << "Server Log: Failed to receive message from client " << i + 1 << std::endl;
            closesocket(current_user);
            break;
        }

        message = std::string(buffer);

        std::cout << "Server Log: Request by client " <<  i + 1 <<" : " << message << std::endl;
        std::string current_expression;
        current_expression = message;
        std::vector<std::string> tokens;
        std::string message_to_send;
        if (check_correct_math_expression(tokens,current_expression)) {
            if (check_zero(tokens)) {
                message_to_send = "Current expression: " + current_expression + " = " + calculate(tokens);
                if (send(current_user, message_to_send.c_str(), message_to_send.size() + 1, 0) == SOCKET_ERROR) {
                    std::cerr << "Server Log: Failed send response" << std::endl;
                    closesocket(current_user);
                    break;
                }
            } else {
                message_to_send = "Divizon by zero detected";
                if (send(current_user, message_to_send.c_str(), message_to_send.size() + 1, 0) == SOCKET_ERROR) {
                    std::cerr << "Server Log: Failed send response" << std::endl;
                    closesocket(current_user);
                    break;
                }
            }
        } else {
            message_to_send = "Incorrect math expression";
            if (send(current_user, message_to_send.c_str(), message_to_send.size() + 1, 0) == SOCKET_ERROR) {
                std::cerr << "Server Log: Failed send response" << std::endl;
                closesocket(current_user);
                break;
            }
        }
        std::cout << "Server Log: Response to client " << i + 1 << " : " << message_to_send <<std::endl;
        current_expression.clear();
    }
    ReleaseSemaphore(m_client_semaphore,1,nullptr);
}

void init() {
    m_client_semaphore = CreateSemaphoreA(nullptr, 2, 2, "process");

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(info::IP);
    addr.sin_port = htons(info::SERVER_PORT);
    addr.sin_family = AF_INET;
    int size = sizeof(addr);

    sock = socket(AF_INET, SOCK_STREAM, NULL);

    if (bind(sock, (SOCKADDR *) &addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Server Log: Failed to bind" << std::endl;
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

    std::cout << "Count of clients:";
    const int count_of_clients = check_correct_count_of_clients();

    std::vector<std::thread> threads;
    for (int i = 0; i < 3; i++) {
        std::cout << "Server Log: Try Client " << i+1 << " to connect" <<std::endl;
        WaitForSingleObject(m_client_semaphore, INFINITE);
        launch_client();
        accept_socket = accept(sock, (SOCKADDR*)&addr, &size);

        if (!accept_socket) {
            std::cerr << "Server Log: Failed to accept client: " << i+1 << std::endl;
            if (closesocket(sock) == SOCKET_ERROR) {
                std::cerr << "Server Log: Failed to terminate connection: " << WSAGetLastError() << std::endl;
            }
            continue;
        }
        run_sockets.push_back(accept_socket);
        if (accept_socket != SOCKET_ERROR) {
            threads.emplace_back(process_client, i);
        }

        std::cout << "Server Log: Client " << i+1 << " connected" << std::endl;
    }


    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "Server Log: All clients disconnected" << std::endl;

    for (auto& socket : run_sockets) {
        closesocket(socket);
    }
    CloseHandle(m_client_semaphore);
    WSACleanup();
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
