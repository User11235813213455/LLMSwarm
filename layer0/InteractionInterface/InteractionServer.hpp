#ifndef INTERACTION_SERVER_HPP_INCLUDED
#define INTERACTION_SERVER_HPP_INCLUDED

#include "network/server.hpp"
#include "../CommonProtocol.hpp"
#include <deque>
#include <optional>

class InteractionServer
{
public:
    /**
     * @brief Constructs a new Interaction server but does not start the server yet
     * 
     */
    InteractionServer();

    /**
     * @brief Initializes the server on the specified port and IP
     * 
     * @param pPort Port on which to listen for new connections
     * @param pIP IP on which to run the server (or empty string to listen on all available interfaces)
     */
    void initialize(unsigned int pPort, std::string pIP="");

    /**
     * @brief Updates the drone positions which the server will transmit to clients
     * 
     * @param pPosition Positions of all drones
     */
    void updateDronePositions(const std::map<uint16_t, Position>& pPosition);

    /**
     * @brief Updates the drone states which the server will transmit to clients
     * 
     * @param pDroneStates States of all drones
     */
    void updateDroneStates(const std::map<uint16_t, DroneState>& pDroneStates);

    /**
     * @brief Updates the drone operations which the server will transmit to the client
     * 
     * @param pDroneOperations The current operation of all drones
     */
    void updateDroneOperations(const std::map<uint16_t, DroneOperation>& pDroneOperations);

    /**
     * @brief Updates the drone swarm state which the server will transmit to the client
     * 
     * @param pSwarmState The current swarm state
     */
    void updateSwarmState(SwarmState pSwarmState);

    /**
     * @brief Returns the current target positions of all drones
     * 
     * @return std::map<uint16_t, Position> Mapping drone ID -> Target Position
     */
    std::map<uint16_t, Position> getTargets();

    /**
     * @brief Returns the next swarm operation which was requested by a client (FIFO) 
     * 
     * @return std::optional<SwarmOperation> The next operation to be executed by the swarm (if any)
     */
    std::optional<SwarmOperation> peekRequest();

    /**
     * @brief Dequeues a request from the requested operations if any request is contained. If no request is contained in the queue, this function does nothing
     * 
     */
    void dequeueRequest();
    
protected:
    /**
     * @brief Sends cyclic notifications to a connected client (if he requested it)
     * 
     * @param pSocket The socket of the connection to the client
     * @param pInterval The interval in which to send out the current state notification in ms
     */
    void notificationThread(int pSocket, uint32_t pInterval);

    /**
     * @brief Handles the communication to a connected client
     * 
     * @param pAddr The address info of the client (IP, ...)
     * @param pSocket The connection socket
     * @return true Continue running
     * @return false Stop running
     */
    bool mainCommunicationThread(sockaddr_in pAddr, int pSocket);

    /**
     * @brief The underlying TCP server object
     * 
     */
    Server* tcpServer;

    /**
     * @brief The current drone positions which were set by updateDronePositions()
     * 
     */
    std::map<uint16_t, Position> dronePositions;

    /**
     * @brief The current drone targets which were set by updateDroneTargets()
     * 
     */
    std::map<uint16_t, Position> droneTargets;

    /**
     * @brief The current drone states which were set by updateDroneStates()
     * 
     */
    std::map<uint16_t, DroneState> droneStates;

    /**
     * @brief The currently active drone operations set by updateDroneOperations()
     * 
     */
    std::map<uint16_t, DroneOperation> droneOperations;
    
    /**
     * @brief The current swarm state set by updateSwarmState()
     * 
     */
    SwarmState swarmState;

    /**
     * @brief A queue storing requests sent by the interaction client (e.g. fast stop, ...)
     * 
     */
    std::deque<SwarmOperation> requestQueue;

    /**
     * @brief A mutex for all attributes of the server linked to sent/received data
     * 
     */
    std::mutex dataMutex;
};

#endif /*INTERACTION_SERVER_HPP_INCLUDED*/