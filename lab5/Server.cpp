#include "info.hpp"

#include <WinSock2.h>
#include <iostream>
#include <list>

SOCKET sock;

std::list<SOCKET> sockets;
HANDLE m_client_mutex;
HANDLE m_semaphore;


void init() {
    m_client_mutex = CreateMutexA(nullptr, false, "client_mutex");
    m_semaphore = CreateSemaphoreA(nullptr, info::MAX_USERS, info::MAX_USERS, "semaphore");

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr(info::IP);
    addr.sin_port = htons(info::SERVER_PORT);
    addr.sin_family = AF_INET;

    sock = socket(AF_INET, SOCK_STREAM, NULL);
    if (bind(sock, (SOCKADDR *) &addr, sizeof(addr) == SOCKET_ERROR)) {
        std::cerr << "Failed to bind" << std::endl;
        closesocket(sock);
        WSACleanup();
        exit(1);
    };
    if (listen(sock, info::MAX_QUEUE_SIZE) == SOCKET_ERROR) {
        std::cerr << "Failed to listen" << std::endl;
        closesocket(sock);
        WSACleanup();
        exit(1);
    }
}

void run() {
    SOCKET accept_socket;

    

}

static DWORD process_client(LPVOID client_param) {
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }
}
