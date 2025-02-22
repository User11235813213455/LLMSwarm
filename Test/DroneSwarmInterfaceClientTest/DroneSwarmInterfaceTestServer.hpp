#ifndef DRONE_SWARM_INTERFACE_TEST_SERVER
#define DRONE_SWARM_INTERFACE_TEST_SERVER

#include "network/server.hpp"
#include "layer0/position.hpp"
#include "layer0/CommonProtocol.hpp"

class DroneSwarmInterfaceTestServer
{
public:
    DroneSwarmInterfaceTestServer(const std::map<uint16_t, Position>& pInitialDronePositions, unsigned short pPort, std::string pIP);

    void update();

    void print();
protected:
    Server* server;

    std::map<uint16_t, Position> initialDronePositions;
    std::map<uint16_t, Position> dronePositions;
    std::map<uint16_t, DroneOperation> droneOperationSetpoint;
    std::map<uint16_t, DroneState> droneStates;
    std::map<uint16_t, Position> targets;
    SwarmState currentSwarmState;

    bool handleCommunication(sockaddr_in pAddr, int pSocket);
    void notificationThread(int pSocket, uint32_t pInterval);

    void handleTakeoffRequest(uint16_t pDrone);
    void handleLandRequest(uint16_t pDrone);
    void handleFastStopRequest(uint16_t pDrone);
    void handleMoveRequest(uint16_t pDrone);

    void handleTakingOffState(uint16_t pDrone);
    void handleLandingState(uint16_t pDrone);
    void handleStoppingState(uint16_t pDrone);
    void handleMovingState(uint16_t pDrone);

    std::mutex dataMutex;
};

#endif /*DRONE_SWARM_INTERFACE_TEST_SERVER*/