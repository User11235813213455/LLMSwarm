/**
 * @file server.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains implementations for TCP servers on windows and linux systems
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifdef linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <future>
#include <math.h>
#include <set>
#include <functional>
#include <map>
#include <algorithm>
#include "server.hpp"
#include <mutex>
#include <thread>

Server::Server(unsigned int pListenPort, std::function<bool(struct sockaddr_in)> pFilter, std::function<bool(struct sockaddr_in, int)> pCommunicationThread, std::string pIP, unsigned int pMaxQueueLength)
{
    this->filter = pFilter;
    this->communicationThread = pCommunicationThread;
    #if _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(1,1), &wsa);
    #endif
    this->listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    const unsigned int flag = 1;
    #ifdef linux
    setsockopt(this->listenSocket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
    #else
    setsockopt(this->listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int));
    #endif
    struct sockaddr_in addr = {0};
    addr.sin_port = htons(pListenPort);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (pIP.empty() ? INADDR_ANY : inet_addr(pIP.c_str()));

    if(bind(this->listenSocket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
    {
        #if _WIN32
        std::cerr << "Can't bind to port " << pListenPort << ": " << WSAGetLastError() << std::endl;
        #endif
        throw(std::runtime_error("Could not bind the listen socket to the specified port for the drone swarm server!"));
    }
    listen(this->listenSocket, pMaxQueueLength);
    this->threadAccept = std::thread(&Server::acceptThread, this);
}
void Server::acceptThread()
{
    while(true)
    {
        struct sockaddr_in addr = {0};
        #ifdef linux
        socklen_t addrLength = sizeof(struct sockaddr_in);
        #else
        int addrLength = sizeof(struct sockaddr_in);
        #endif
        int c = accept(this->listenSocket, (struct sockaddr*)&addr, &addrLength);
        if(this->filter(addr))
        {
            std::thread t = std::thread([=, this] {
                while(this->communicationThread(addr, c));
            });
            t.detach();
        }
        else
        {
            #ifdef linux
            close(c);
            #else
            closesocket(c);
            #endif
        }
    }
}