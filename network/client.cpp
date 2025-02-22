#include "client.hpp"
#ifdef linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#endif
#include <stdexcept>
#include <thread>

Client::Client(unsigned int pPort, std::function<bool(struct sockaddr_in, int)> pCommunicationHandler, std::string pIP)
{
    this->communicationHandler = pCommunicationHandler;
    #if _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(1,1), &wsa);
    #endif
    this->connectedSocket = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
    const unsigned int flag = 1;
    #ifdef linux
    setsockopt(this->connectedSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    #else
    setsockopt(this->connectedSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int));
    #endif
    struct sockaddr_in addr = {0};
    addr.sin_port = htons(pPort);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (pIP.empty() ? inet_addr("127.0.0.1") : inet_addr(pIP.c_str()));

    if(connect(this->connectedSocket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
    {
        #if _WIN32
        std::cerr << "Can't connect to port " << pPort << ": " << WSAGetLastError() << std::endl;
        #endif
        throw(std::runtime_error("Could not connect the socket to the specified port and IP combination!"));
    }
    this->communicationThread = std::thread(pCommunicationHandler, addr, connectedSocket);
}