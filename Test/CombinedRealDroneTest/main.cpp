#include "layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.hpp"
#include "layer0/InteractionInterface/InteractionServer.hpp"
#include "layer0/GeometryModule/GeometryModule.hpp"
#include "layer1/SwarmOperationHandler/SwarmOperationHandler.hpp"
#include "../InteractionInterfaceServerTest/InteractionInterfaceTestClient.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <random>

void printDroneSwarmInterfaceClientInformation(InteractionInterfaceTestClient& client)
{
    std::map<uint16_t, Position> currentPositions = client.getPositions();
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
}


void InteractionInterfaceClientThread()
{
    InteractionInterfaceTestClient client;
    client.initialize(12346, "127.0.0.1");

    while(true)
    {
        std::cout << "Please enter your command:" << std::endl
                  << "\t0: Takeoff" << std::endl
                  << "\t1: Move" << std::endl
                  << "\t2: Land" << std::endl
                  << "\t3: Print swarm state" << std::endl
                  << "\t4: Fast stop" << std::endl 
                  << "\t5: Switch positions" << std::endl
                  << "\t6: Move to random positions" << std::endl << std::endl;

        try
        {
            int a;
            std::cin >> a;

            switch(a)
            {
                case 0:
                {
                    std::cout << "Confirm takeoff command? (y/n) ";
                    char c;
                    try
                    {
                        std::cin >> c;
                        if(c == 'y')
                        {
                            std::map<uint16_t, Position> positions = client.getPositions();
                            std::map<uint16_t, Position> takeoffTargets;
                            for(const auto& p : positions)
                            {
                                takeoffTargets.insert(std::make_pair(p.first, Position(p.second.getX(), p.second.getY(), p.second.getZ() + 0.6, 0.0)));
                            }
                            client.updateTargets(takeoffTargets);
                            client.requestOperation(SWARM_OPERATION_TAKEOFF);
                        }
                    }
                    catch(...)
                    {
                        MSG_ERROR("Invalid input");
                    }
                }
                break;
                case 1:
                {
                    std::cout << "Currently the drones are located at:" << std::endl;
                    std::map<uint16_t, Position> positions = client.getPositions();
                    std::map<uint16_t, Position> targets = positions;

                    for(const auto& p : positions)
                    {
                        std::cout << "\t" << p.first << ": (x=" << p.second.getX() << ", y=" << p.second.getY() << ", z=" << p.second.getZ() << ")" << std::endl;
                    }
                    std::cout << std::endl;
                    for(const auto& p : positions)
                    {
                        std::cout << "Please define the target coordinates for drone " << p.first << ": " << "(x=";
                        try
                        {
                            double x;
                            std::cin >> x;
                            std::cout << ", y=";
                            double y;
                            std::cin >> y;
                            std::cout << ", z=";
                            double z;
                            std::cin >> z;
                            std::cout << ")" << std::endl;
                            targets.at(p.first) = Position(x, y, z, 0.0);
                        }
                        catch(...)
                        {
                            MSG_WARNING("Setpoint will be set to current position");
                        }
                    }
                    std::cout << "Confirm the movement command? (y/n) ";
                    char c;
                    try
                    {
                        std::cin >> c;
                        if(c == 'y')
                        {
                            client.updateTargets(targets);
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            client.requestOperation(SWARM_OPERATION_MOVE);
                        }
                    }
                    catch(...)
                    {
                        MSG_ERROR("Invalid input");
                    }
                }
                break;
                case 2:
                {
                    std::cout << "Confirm landing command? (y/n) ";
                    char c;
                    try
                    {
                        std::cin >> c;
                        if(c == 'y')
                        {
                            client.requestOperation(SWARM_OPERATION_LAND);
                        }
                    }
                    catch(...)
                    {
                        MSG_ERROR("Invalid input");
                    }
                }
                break;
                case 3:
                {
                    printDroneSwarmInterfaceClientInformation(client);
                }
                break;
                case 4:
                {
                    client.requestOperation(SWARM_OPERATION_FAST_STOP);
                }
                break;
                case 5:
                {
                    std::cout << "Currently the drones are located at:" << std::endl;
                    std::map<uint16_t, Position> positions = client.getPositions();
                    std::map<uint16_t, Position> targets = positions;

                    for(const auto& p : positions)
                    {
                        std::cout << "\t" << p.first << ": (x=" << p.second.getX() << ", y=" << p.second.getY() << ", z=" << p.second.getZ() << ")" << std::endl;
                    }
                    std::cout << std::endl;

                    std::vector<Position> mixVector;
                    for(const auto& p : positions)
                    {
                        mixVector.push_back(p.second);
                    }
                    
                    std::vector<Position>::reverse_iterator i = mixVector.rbegin();

                    for(const auto& p : positions)
                    {
                        targets.at(p.first) = *i;
                        i++;
                    }

                    std::cout << "Confirm the movement command? (y/n) ";
                    char c;
                    try
                    {
                        std::cin >> c;
                        if(c == 'y')
                        {
                            client.updateTargets(targets);
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            client.requestOperation(SWARM_OPERATION_MOVE);
                        }
                    }
                    catch(...)
                    {
                        MSG_ERROR("Invalid input");
                    }
                }
                break;
                case 6:
                {
                    std::cout << "Currently the drones are located at:" << std::endl;
                    std::map<uint16_t, Position> positions = client.getPositions();
                    std::map<uint16_t, Position> targets = positions;

                    for(const auto& p : positions)
                    {
                        std::cout << "\t" << p.first << ": (x=" << p.second.getX() << ", y=" << p.second.getY() << ", z=" << p.second.getZ() << ")" << std::endl;
                    }
                    std::cout << std::endl;

                    for(const auto& p : positions)
                    {
                        targets.at(p.first) = Position(p.second.getX() + rand() % 1000 / 999.0 * 60, p.second.getY() + rand() % 1000 / 999.0 * 60, p.second.getZ() + rand() % 1000 / 999.0 * 60, 0.0);
                    }

                    std::cout << "Confirm the movement command? (y/n) ";
                    char c;
                    try
                    {
                        std::cin >> c;
                        if(c == 'y')
                        {
                            client.updateTargets(targets);
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            client.requestOperation(SWARM_OPERATION_MOVE);
                        }
                    }
                    catch(...)
                    {
                        MSG_ERROR("Invalid input");
                    }
                }
                break;
                default:
                {
                    MSG_ERROR("Invalid input!");
                }
            }
        }
        catch(...)
        {
            /*Ignore*/
            MSG_ERROR("Invalid input!");
        }
    }
}
int main()
{
    //MSG_INFO("Starting the drone swarm interface...");
    //#if linux
    //system("python3 ../droneSwarmInterface.py &");
    //#else
    //system("start /b python3 ../droneSwarmInterface.py");
    //#endif
    //MSG_INFO("Wait for startup (7s)...");
    //std::this_thread::sleep_for(std::chrono::milliseconds(7000));

    SwarmOperationHandler* swarmOperationHandler = nullptr;
    GeometryModule* geometryModule = nullptr;
    InteractionServer* interactionServer;

    bool init = false;

    MSG_INFO("Start up drone swarm interface client...");
    DroneSwarmInterfaceClient client;
    client.initialize(12345, "127.0.0.1");

    MSG_INFO("Waiting for first state message:");

    while(true)
    {
        switch(client.getClientState())
        {
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_NOT_INITIALIZED:
            {
                MSG_INFO("\tClient not initialized (yet)");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_INITIALIZED:
            {
                MSG_INFO("\tClient initialized");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_REGISTER_NOTIFICATION:
            {
                MSG_INFO("\tClient registering notification");
            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_RUNNING:
            { 
                if(!init)
                {
                    /*Init server after connecting to the client*/
                    interactionServer = new InteractionServer();
                    interactionServer->initialize(12346);
                    geometryModule = new GeometryModule(0.6, 0.6, Position(0.2, 0.2, 0.7, 0.0), Position(0.2, 0.21, 0.7, 0.0), client.getDronePositions());
                    swarmOperationHandler = new SwarmOperationHandler(*interactionServer, client, *geometryModule);
                    new std::thread(InteractionInterfaceClientThread);
                    init = true;
                }

            }
            break;
            case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED:
            {
                MSG_INFO("Client disconnected");
                delete interactionServer;
                delete geometryModule;
                delete swarmOperationHandler;
                std::cout << "Exited gracefully" << std::endl;
                std::exit(0);
            }
            break;
        }

        if(init)
        {
            swarmOperationHandler->update();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}