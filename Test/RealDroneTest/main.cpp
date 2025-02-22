#include "layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <iostream>

int main()
{
    DroneSwarmInterfaceClient client;
    client.initialize(12346, "127.0.0.1");

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
    uint32_t startTime;

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
                std::map<uint16_t, Position> currentPositions = client.getDronePositions();
                std::map<uint16_t, DroneState> currentStates = client.getDroneStates();
                SwarmState swarmState = client.getSwarmState();

                std::cout << "Drone positions: ";
                for(const auto& p : currentPositions)
                {
                    std::cout << "\t" << p.first << " at " << (std::string)p.second << std::endl;
                }
                std::cout << "Drone states: ";
                for(const auto& p : currentStates)
                {
                    std::cout << "\t" << p.first << ": ";
                    switch(p.second)
                    {
                        case DRONE_IDLE:
                        {
                            std::cout << "DRONE_IDLE";
                        }
                        break;
                        case DRONE_HOVERING:
                        {
                            std::cout << "DRONE_HOVERING";
                        }
                        break;
                        case DRONE_MOVING:
                        {
                            std::cout << "DRONE_MOVING";
                        }
                        break;
                        case DRONE_LANDING:
                        {
                            std::cout << "DRONE_LANDING";
                        }
                        break;
                        case DRONE_TAKING_OFF:
                        {
                            std::cout << "DRONE_TAKING_OFF";
                        }
                        break;
                        case DRONE_STOPPING:
                        {
                            std::cout << "DRONE_STOPPING";
                        }
                        break;
                        default:
                        {
                            std::cout << p.second  << ": "<< "INVALID STATE!!!";
                        }
                    }
                    std::cout << std::endl;
                }
                std::cout << "Swarm state: ";
                switch(swarmState)
                {
                    case SWARM_IDLE:
                    {
                        std::cout << "SWARM_IDLE";
                    }
                    break;
                    case SWARM_HOVERING:
                    {
                        std::cout << "SWARM_HOVERING";
                    }
                    break;
                    case SWARM_LANDING:
                    {
                        std::cout << "SWARM_LANDING";
                    }
                    break;
                    case SWARM_TAKING_OFF:
                    {
                        std::cout << "SWARM_TAKING_OFF";
                    }
                    break;
                    case SWARM_MOVING:
                    {
                        std::cout << "SWARM_MOVING";
                    }
                    break;
                    case SWARM_STOPPING:
                    {
                        std::cout << "SWARM_STOPPING";
                    }
                    break;
                    default:
                    {
                        std::cout << "(INVALID STATE!!!)";
                    }
                }
                std::cout << std::endl;


                if(step == 0)
                {
                    MSG_INFO("Client running");
                }
                switch(step)
                {
                    case 0:
                    case 5:
                    {
                        std::cout << "When you are ready to start the takeoff operation please press Enter..." << std::endl;
                        std::string a;
                        std::cin >> a;
                        MSG_INFO("Start takeoff...");
                        std::map<uint16_t, Position> takeoffPositions;
                        std::map<uint16_t, DroneOperation> droneOperations;

                        for(const auto& p : currentPositions)
                        {
                            takeoffPositions.insert(std::make_pair(p.first, p.second + Position(0.0, 0.0, 0.7, 0.0)));
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
                            MSG_INFO("Move swarm...");
                            std::map<uint16_t, Position> targets;
                            std::map<uint16_t, DroneOperation> droneOperations;

                            for(const auto& p : currentPositions)
                            {
                                targets.insert(std::make_pair(p.first, p.second + Position((rand() % 100) / 999.0, (rand() % 100) / 999.0, (rand() % 100) / 999.0, 0.0)));
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

                            for(const auto& p : currentPositions)
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
                            step = 10;
                        }
                    }
                    break;
                    case 8:
                    {
                        if(client.getSwarmState() == SWARM_HOVERING)
                        {
                            MSG_INFO("Reached random targets. Test fast stop in 3s...");
                            startTime = getMillis();
                            step++;
                        }
                    }
                    break;
                    case 9:
                    {
                        if(getTimedif(startTime, getMillis()) > 3000)
                        {
                            MSG_INFO("Fast stop!");
                            std::map<uint16_t, DroneOperation> droneOperations;

                            for(const auto& p : currentPositions)
                            {
                                droneOperations.insert(std::make_pair(p.first, DRONE_OPERATION_FAST_STOP));
                            }

                            client.updateDroneOperations(droneOperations);
                            step++;
                        }
                    }
                    break;
                    case 10:
                    {
                        if(client.getSwarmState() == SWARM_IDLE)
                        {
                            MSG_INFO("Test successful. Quit...");
                            std::exit(0);
                        }
                    }
                    break;
                }
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED:
            {
                MSG_INFO("Client disconnected");
            }
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}