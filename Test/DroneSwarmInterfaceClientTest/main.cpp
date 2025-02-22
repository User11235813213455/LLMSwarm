#include "DroneSwarmInterfaceTestServer.hpp"
#include "layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.hpp"
#include "layer0/InteractionInterface/InteractionServer.hpp"
#include "logger.hpp"

int main()
{
    std::map<uint16_t, Position> initialPositions = {
        {0, Position(0.0, 0.0, 0.0, 0.0)},
        {1, Position(10.0, 10.0, 0.0, 0.0)},
        {2, Position(5.0, 5.0, 0.0, 0.0)}
    };
    DroneSwarmInterfaceTestServer server(initialPositions, 12345, "");
    DroneSwarmInterfaceClient client;
    client.initialize(12345, "127.0.0.1");

    bool exc = false;

    try
    {
        client.getDronePositions();
    }
    catch(...)
    {
        exc = true;
        MSG_INFO("Client throws exception before first update (as it should)");
    }
    if(!exc)
    {
        MSG_ERROR("Client does not throw exception when trying to access non-present position data!");
    }

    unsigned int step = 0;

    unsigned int cntr = 0;

    while(true)
    {
        switch(client.getClientState())
        {
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_NOT_INITIALIZED:
            {
                MSG_INFO("Client not initialized (yet)");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_INITIALIZED:
            {
                MSG_INFO("Client initialized");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_REGISTER_NOTIFICATION:
            {
                MSG_INFO("Client registering notification");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_RUNNING:
            { 
                if(step == 0)
                {
                    MSG_INFO("Client running");
                }
                switch(step)
                {
                    case 0:
                    case 5:
                    {
                        MSG_INFO("Start takeoff...");
                        std::map<uint16_t, Position> takeoffPositions;
                        std::map<uint16_t, DroneOperation> droneOperations;

                        for(const auto& p : initialPositions)
                        {
                            takeoffPositions.insert(std::make_pair(p.first, p.second + Position(0.0, 0.0, 1.0, 0.0)));
                            droneOperations.insert(std::make_pair(p.first, DRONE_OPERATION_TAKE_OFF));
                        }
                        client.updateDroneTargets(takeoffPositions);
                        client.updateDroneOperations(droneOperations);
                        step++;
                    }
                    break;
                    case 6:
                    case 1:
                    {
                        if(client.getSwarmState() == SWARM_HOVERING)
                        {
                            MSG_INFO("Move swarm to random positions...");
                            std::map<uint16_t, Position> targets;
                            std::map<uint16_t, DroneOperation> droneOperations;

                            for(const auto& p : initialPositions)
                            {
                                targets.insert(std::make_pair(p.first, Position((rand() % 1000) / 999.0, (rand() % 1000) / 999.0, (rand() % 1000) / 999.0, 0.0)));
                                droneOperations.insert(std::make_pair(p.first, DRONE_OPERATION_MOVE));
                            }

                            client.updateDroneTargets(targets);
                            client.updateDroneOperations(droneOperations);

                            step++;
                        }
                    }
                    break;
                    case 7:
                    case 2:
                    {
                        if(client.getSwarmState() != SWARM_HOVERING)
                        {
                            step++;
                        }
                    }
                    break;
                    case 3:
                    {
                        if(client.getSwarmState() == SWARM_HOVERING)
                        {
                            MSG_INFO("Reached random targets. Test landing...");
                            std::map<uint16_t, DroneOperation> droneOperations;

                            for(const auto& p : initialPositions)
                            {
                                droneOperations.insert(std::make_pair(p.first, DRONE_OPERATION_LAND));
                            }

                            client.updateDroneOperations(droneOperations);
                            step++;
                        }
                    }
                    break;
                    case 4:
                    {
                        if(client.getSwarmState() == SWARM_IDLE)
                        {
                            MSG_INFO("Landing successfull. Take off again...");
                            step++;
                        }
                    }
                    break;
                    case 8:
                    {
                        if(client.getSwarmState() == SWARM_HOVERING)
                        {
                            MSG_INFO("Reached random targets. Test fast stop...");
                            std::map<uint16_t, DroneOperation> droneOperations;

                            for(const auto& p : initialPositions)
                            {
                                droneOperations.insert(std::make_pair(p.first, DRONE_OPERATION_FAST_STOP));
                            }

                            client.updateDroneOperations(droneOperations);
                            step++;
                        }
                    }
                    break;
                    case 9:
                    {
                        MSG_INFO("Test successful. Quit...");
                        std::exit(0);
                    }
                }
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED:
            {
                MSG_INFO("Client disconnected");
            }
            break;
        }
        server.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));

        cntr++;
        if(cntr == 30)
        {
            server.print();
            cntr = 0;
        }
    }
}