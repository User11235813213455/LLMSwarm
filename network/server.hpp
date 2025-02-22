/**
 * @file server.hpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains data structure and function declarations to create TCP servers and manage their connections
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef SERVER_HPP_INCLUDED
#define SERVER_HPP_INCLUDED

#ifdef linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <future>
#include <math.h>
#include <set>
#include <functional>
#include <map>
#include <thread>
#include <mutex>

class Server
{
public:
    /**
     * @brief Creates a new TCP server
     * 
     * @param pListenPort The port on which to listen for new connections
     * @param pFilter A filter function that is supposed to return true if a connection from a client specified in the sockaddr_in structure shall be accepted and false otherwise
     * @param pCommunicationThread A function which manages the communication to the connected clients. The function will be called in a seperate detached thread until it returns false.
     * Its first argument is the sockaddr_in structure from the accept call (containing client information) and its second parameter is the socket of the connection.
     * @param pIP The IP on which to listen for new connections or an empty string to listen on all IPs on all interfaces
     * @param pMaxQueueLength The maximum length of the queue with pending connection requests (for the listen call)
     */
    Server(unsigned int pListenPort, std::function<bool(struct sockaddr_in)> pFilter, std::function<bool(struct sockaddr_in, int)> pCommunicationThread, std::string pIP="", unsigned int pMaxQueueLength=SOMAXCONN);
private:
    /**
     * @brief A function which runs in a separate thread and manages incoming connection requests. It decides if it shall connect by calling the user provided filter function
     */
    void acceptThread();

    /**
     * @brief The socket on which the server is listening
     */
    int listenSocket;

    /**
     * @brief The handle to the thread in which the server accepts new connection requests (or declines them)
     */
    std::thread threadAccept;

    /**
     * @brief A user-provided filter function which filters new connection requests
     */
    std::function<bool(struct sockaddr_in)> filter;

    /**
     * @brief A user-provided function which manages connections
     */
    std::function<bool(struct sockaddr_in, unsigned int)> communicationThread;
};

#endif /*SERVER_HPP_INCLUDED*/