#ifndef DRONE_SWARM_INTERFACE_CLIENT_HPP_INCLUDED
#define DRONE_SWARM_INTERFACE_CLIENT_HPP_INCLUDED

#include "network/client.hpp"
#include <map>
#include "../position.hpp"
#include "../CommonProtocol.hpp"
#include <mutex>
#include <set>
#include <optional>

class DroneSwarmInterfaceClient
{
public:
    /**
     * @brief Possible states of the drone swarm interface client
     * 
     */
    enum ClientState
    {
        /**
         * @brief This state is entered before the client is connected (at the constructor call)
         * 
         */
        DRONE_SWARM_INTERFACE_CLIENT_NOT_INITIALIZED,

        /**
         * @brief This state means that the initialization has finished and was successful
         * 
         */
        DRONE_SWARM_INTERFACE_CLIENT_INITIALIZED,

        /**
         * @brief This state means that the client sent a request for drone state notifications and is waiting for the first notification message
         * 
         */
        DRONE_SWARM_INTERFACE_CLIENT_REGISTER_NOTIFICATION,

        /**
         * @brief In this state the client knows the state, position, ... of all drones and can request operations
         * 
         */
        DRONE_SWARM_INTERFACE_CLIENT_RUNNING,

        /**
         * @brief This indicates that the connection to the server was closed
         * 
         */
        DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED,
    };

    /**
     * @brief Construct a new drone swarm interface client but does not initialize the client yet
     * 
     */
    DroneSwarmInterfaceClient();

    /**
     * @brief Initializes the client by connecting to the specified port and IP
     * 
     * @param pPort Port to connect to
     * @param pIP IP of the server
     */
    void initialize(unsigned int pPort, std::string pIP);

    /**
     * @brief Return a set of all reported drones
     * 
     * @return std::set<uint16_t> Set of reported drones
     */
    std::set<uint16_t> getDrones();

    /**
     * @brief Returns the current client state
     * 
     * @return ClientState Current client state
     */
    ClientState getClientState();

    /**
     * @brief Returns the last reported drone positions
     * 
     * @return std::map<uint16_t, Position> Mapping drone ID -> Position
     */
    std::map<uint16_t, Position> getDronePositions();

    /**
     * @brief Returns the last reported drone states
     * 
     * @return std::map<uint16_t, DroneState> Mapping drone ID -> DroneState
     */
    std::map<uint16_t, DroneState> getDroneStates();

    /**
     * @brief Returns the last reported swarm state
     * 
     * @return SwarmState The last reported swarm state
     */
    SwarmState getSwarmState();

    /**
     * @brief Updates the target positions for all drones
     * 
     * @param pTargets Mapping drone ID -> target position
     */
    void updateDroneTargets(const std::map<uint16_t, Position>& pTargets);

    /**
     * @brief Updates the drone operations
     * 
     * @param pOperations 
     */
    void updateDroneOperations(const std::map<uint16_t, DroneOperation>& pOperations);
protected:

    /**
     * @brief Handles the communication to the drone controller server
     * 
     * @param pAddr The address information of the connection to the server (IP, Port, ...)
     * @param pSocket The connected socket
     * @return true Keep running
     * @return false Stop
     */
    bool communicationHandler(struct sockaddr_in pAddr, int pSocket);

    /**
     * @brief The underlying TCP client used
     * 
     */
    Client* tcpClient;

    /**
     * @brief The last reported positions of all drones
     * 
     */
    std::map<uint16_t, Position> dronePositions;

    /**
     * @brief The targets which will be sent to the server
     * 
     */
    std::optional<std::map<uint16_t, Position>> droneTargets;

    /**
     * @brief The last reported states of all drones
     * 
     */
    std::map<uint16_t, DroneState> droneStates;

    /**
     * @brief The last reported state of the swarm
     */
    SwarmState swarmState;

    /**
     * @brief The operations for all drones which will be sent to the server
     * 
     */
    std::optional<std::map<uint16_t, DroneOperation>> droneOperations;

    /**
     * @brief The current state of the client
     * 
     */
    ClientState state;

    /**
     * @brief A mutex protecting the data attributes (targets, positions, ...)
     * 
     */
    std::mutex dataMutex;
};

#endif /*DRONE_SWARM_INTERFACE_CLIENT_HPP_INCLUDED*/