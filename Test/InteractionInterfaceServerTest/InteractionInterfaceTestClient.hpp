#ifndef INTERACTION_INTERFACE_TEST_CLIENT_HPP_INCLUDED
#define INTERACTION_INTERFACE_TEST_CLIENT_HPP_INCLUDED

#include "layer0/position.hpp"
#include "layer0/CommonProtocol.hpp"
#include <optional>
#include <mutex>
#include "network/client.hpp"
#include "network/protocol.hpp"

class InteractionInterfaceTestClient
{
public:
     enum ClientState
    {
        INTERACTION_INTERFACE_TEST_CLIENT_NOT_INITIALIZED,
        INTERACTION_INTERFACE_TEST_CLIENT_INITIALIZED,
        INTERACTION_INTERFACE_TEST_CLIENT_REGISTER_NOTIFICATION,
        INTERACTION_INTERFACE_TEST_CLIENT_RUNNING,
        INTERACTION_INTERFACE_TEST_CLIENT_DISCONNECTED,
    };

    InteractionInterfaceTestClient();

    void initialize(int pPort, std::string pIP);

    std::map<uint16_t, Position> getPositions();
    std::map<uint16_t, DroneState> getDroneStates();
    std::map<uint16_t, DroneOperation> getDroneOperations();
    SwarmState getSwarmState();

    void requestOperation(SwarmOperation pOperation);
    void updateTargets(const std::map<uint16_t, Position>& pTargets);

    ClientState getClientState();

protected:
    bool communicationHandler(struct sockaddr_in pAddr, int pSocket);

    std::map<uint16_t, Position> positions;
    std::optional<std::map<uint16_t, Position>> targets;
    std::map<uint16_t, DroneState> droneStates;
    std::map<uint16_t, DroneOperation> droneOperations;
    SwarmState swarmState;
    std::optional<SwarmOperation> operation;
    std::mutex dataMutex;

    Client* tcpClient;
    ClientState state;
};

#endif /*INTERACTION_INTERFACE_CLIENT_HPP_INCLUDED*/