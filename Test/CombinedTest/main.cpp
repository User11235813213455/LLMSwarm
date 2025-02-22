#include "../DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.hpp"
#include "../InteractionInterfaceServerTest/InteractionInterfaceTestClient.hpp"
#include "layer1/SwarmOperationHandler/SwarmOperationHandler.hpp"
#include "logger.hpp"
#include "utils.hpp"

int main()
{
    InteractionServer interactionServer;
    DroneSwarmInterfaceClient client;

    std::map<uint16_t, Position> initialPositions = {
        {0, Position(0.0, 0.0, 0.0, 0.0)},
        {1, Position(-0.13, 1.1, -0.01, 0.0)},
        {2, Position(2.1, -1.0, 0.0, 0.0)}
    };

    GeometryModule geometry(1.0, 2.0, Position(0.2, 0.2, 0.2, 0.0), Position(0.2, 0.2, 0.2, 0.0), initialPositions);

    DroneSwarmInterfaceTestServer testServer(initialPositions, 12346, "");
    InteractionInterfaceTestClient testClient;

    interactionServer.initialize(12345);
    testClient.initialize(12345, "127.0.0.1");
    client.initialize(12346, "127.0.0.1");

    SwarmOperationHandler opHandler(interactionServer, client, geometry);

    unsigned int step = 0;

    unsigned int cntr = 0;

    while(client.getClientState() != DroneSwarmInterfaceClient::ClientState::DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        MSG_INFO("Waiting for drone swarm interface client...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    while(testClient.getClientState() != InteractionInterfaceTestClient::ClientState::INTERACTION_INTERFACE_TEST_CLIENT_RUNNING)
    {
        MSG_INFO("Waiting for interaction client...");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    uint32_t entryTime;

    while(true)
    {
        switch(step)
        {
            case 0:
            {
                testClient.requestOperation(SWARM_OPERATION_TAKEOFF);
                step++;
            }
            break;
            case 1:
            {
                if(testClient.getSwarmState() == SWARM_HOVERING)
                {
                    /*Continue*/
                    std::map<uint16_t, Position> targets;

                    for(const auto& p : initialPositions)
                    {
                        targets.insert(std::make_pair(p.first, Position((rand() % 1000) / 99.0, (rand() % 1000) / 99.0, (rand() % 1000) / 99.0, 0.0)));
                    }
                    testClient.updateTargets(targets);
                    testClient.requestOperation(SWARM_OPERATION_MOVE);
                    step++;
                }
            }
            break;
            case 2:
            {
                if(testClient.getSwarmState() == SWARM_HOVERING)
                {
                    entryTime = getMillis();
                    step++;
                }
            }
            break;
            case 3:
            {
                if(testClient.getSwarmState() != SWARM_HOVERING)
                {
                    step--;
                }
                else
                {
                    if(getTimedif(entryTime, getMillis()) >= 1000)
                    {
                        step++;
                    }
                }
            }
            break;
            case 4:
            {
                testClient.requestOperation(SWARM_OPERATION_FAST_STOP);
                step++;
            }
            break;
            case 5:
            {
                if(testClient.getSwarmState() == SWARM_IDLE)
                {
                    MSG_INFO("Test successful!\n");
                    exit(0);
                }
            }
            break;
        }
        

        cntr++;
        if(cntr == 30)
        {
            testServer.print();
            cntr = 0;
        }

        testServer.update();
        opHandler.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}