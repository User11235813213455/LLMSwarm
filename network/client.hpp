#ifndef CLIENT_HPP_INCLUDED
#define CLIENT_HPP_INCLUDED

#include <functional>
#include <string>
#include <thread>
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

class Client
{
public:
    /**
     * @brief Creates a new Client based on the port and IP of a server and a communication handler
     * @param pPort The port on which the client shall connect to the server
     * @param pIP The IP to which the client shall be connected
     * @param pCommunicationHandler A function which handles the actual communication between the client and the server. It takes a sockaddr_in structure of the connection (after it was established), which containes e.g. the IP and the connected socket and returns after the communication is finished
     */
    Client(unsigned int pPort, std::function<bool(struct sockaddr_in, int)> pCommunicationHandler, std::string pIP);
protected:
    /*The connected socket*/
    int connectedSocket;

    /*The communication handler, i.e. the function which will be called in a seperate thread to handle the actual communication*/
    std::function<bool(struct sockaddr_in, int)> communicationHandler;

    /*The thread which calls the communicationHandler function*/
    std::thread communicationThread;
};

#endif /*CLIENT_HPP_INCLUDED*/