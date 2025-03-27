#include<WinSock2.h>

namespace info{
    const static char* IP = "127.0.0.1";
    const static short SERVER_PORT = 1488;
    constexpr int MAX_QUEUE_SIZE = SOMAXCONN;
    const static int MAX_USERS = 2;
}