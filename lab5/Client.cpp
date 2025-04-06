#include <winsock2.h>
#include <iostream>

#include "info.hpp"


std::string from_server(SOCKET socket) {
    std::string message;
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    if (recv(socket, buffer, sizeof(buffer) - 1, 0) == -1) {
        std::cerr << "Server disconnect" << std::endl;
        if(closesocket(socket) == SOCKET_ERROR){
            std::cerr << "Failed close socket" << std::endl;
        };
        WSACleanup();
        exit(1);

    }
    message = std::string(buffer);

    return message;

}

void to_Server(std::string message,SOCKET socket) {

    if (send(socket, message.c_str(),strlen(message.c_str()), 0) == SOCKET_ERROR) {
        std::cerr << "Server disconnect";
        if(closesocket(socket) == SOCKET_ERROR){
            std::cerr << "Failed close socket" << std::endl;
        };
        WSACleanup();
        exit(1);
    }
}


int main() {
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cerr << "Error: failed to link library.\n";
        return 1;
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(info::IP);
    addr.sin_port = htons(info::SERVER_PORT);
    addr.sin_family = AF_INET;
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        if(closesocket(sock) == SOCKET_ERROR){
            std::cerr << "Failed close socket" << std::endl;
        };
        WSACleanup();
        std::cerr << "Error: failed to connect.\n";
        return 1;
    }

    std::cout << "Success connect to server.\n";

    while (true) {
        std::string message;
        std::cin >> message;
        to_Server(message,sock);
        std::string from = from_server(sock);
        std::cout << from << "\n";
    }

}