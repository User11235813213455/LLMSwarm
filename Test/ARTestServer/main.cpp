#include "../DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.hpp"
#include "layer1/SwarmOperationHandler/SwarmOperationHandler.hpp"
#include "logger.hpp"
#include "utils.hpp"

int main()
{
    InteractionServer interactionServer;
    DroneSwarmInterfaceClient client;

    std::map<uint16_t, Position> initialPositions = {
        {0, Position(0, 0, 0.0, 0.0)},
        {1, Position(4, 4, 0.0, 0.0)},
        {2, Position(1, 1, 0.0, 0.0)},
        {3, Position(2, 1, 0.0, 0.0)},
        {4, Position(1, 2, 0.0, 0.0)},
        /*{5, Position(4, 2, 0.0, 0.0)},
        {6, Position(5, 2, 0.0, 0.0)},
        {7, Position(6, 1, 0.0, 0.0)}*/      
    };
    std::map<uint16_t, DroneOperation> droneOps = {
        {0, DRONE_OPERATION_NONE},
        {1, DRONE_OPERATION_NONE},
        {2, DRONE_OPERATION_NONE},
        {3, DRONE_OPERATION_NONE},
        {4, DRONE_OPERATION_NONE},
        /*{5, DRONE_OPERATION_NONE},
        {6, DRONE_OPERATION_NONE},
        {7, DRONE_OPERATION_NONE},*/  
    };

    GeometryModule geometry(1.0, 0.7, Position(0.6, 0.6, 0.6, 0.0), Position(0.2, 0.21, 0.4, 0.0), initialPositions);

    DroneSwarmInterfaceTestServer testServer(initialPositions, 12345, "");
    client.initialize(12345, "127.0.0.1");

    unsigned int cntr = 0;

    while(client.getClientState() != DroneSwarmInterfaceClient::ClientState::DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        MSG_INFO("Waiting for drone swarm interface client...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    interactionServer.updateDronePositions(client.getDronePositions());
    interactionServer.updateDroneStates(client.getDroneStates());
    interactionServer.updateDroneOperations(droneOps);
    interactionServer.updateSwarmState(SWARM_IDLE);
    interactionServer.initialize(12346);

    SwarmOperationHandler opHandler(interactionServer, client, geometry);

    while(true)
    {
        cntr++;
        if(cntr == 30)
        {
            testServer.print();
            cntr = 0;
        }

        testServer.update();
        opHandler.update();

        std::map<uint16_t, Position> m = client.getDronePositions();
        for (const auto& c : m)
        {
            for (const auto& c2 : m)
            {
                if (c.first == c2.first)
                {
                    continue;
                }
                else
                {
                    if (c.second.getEuclideanDistance(c2.second) < 0.2)
                    {
                        exit(0);
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}