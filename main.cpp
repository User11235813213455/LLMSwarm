#include "layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.hpp"
#include "layer0/InteractionInterface/InteractionServer.hpp"
#include "layer0/GeometryModule/GeometryModule.hpp"
#include "layer1/SwarmOperationHandler/SwarmOperationHandler.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <random>

void printDroneSwarmInterfaceClientInformation(DroneSwarmInterfaceClient& client)
{
    std::map<uint16_t, Position> currentPositions = client.getDronePositions();
    std::map<uint16_t, DroneState> currentStates = client.getDroneStates();
    SwarmState swarmState = client.getSwarmState();

    std::cout << "Drone positions: ";
    for (const auto& p : currentPositions)
    {
        std::cout << "\t" << p.first << " at " << (std::string)p.second << std::endl;
    }
    std::cout << "Drone states: ";
    for (const auto& p : currentStates)
    {
        std::cout << "\t" << p.first << ": ";
        switch (p.second)
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
            std::cout << p.second << ": " << "INVALID STATE!!!";
        }
        }
        std::cout << std::endl;
    }
    std::cout << "Swarm state: ";
    switch (swarmState)
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

int main()
{
    SwarmOperationHandler* swarmOperationHandler = nullptr;
    GeometryModule* geometryModule = nullptr;
    InteractionServer* interactionServer;
    uint32_t lastPrintTime = getMillis();

    bool init = false;

    MSG_INFO("Start up drone swarm interface client...");
    DroneSwarmInterfaceClient client;
    client.initialize(12345, "127.0.0.1");

    MSG_INFO("Waiting for first state message:");

    while (true)
    {
        switch (client.getClientState())
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
            if (!init)
            {
                /*Init server after connecting to the client*/
                MSG_INFO("Starting up interaction server...");

                interactionServer = new InteractionServer();
                interactionServer->updateDronePositions(client.getDronePositions());
                interactionServer->updateDroneStates(client.getDroneStates());
                std::map<uint16_t, DroneOperation> operations;
                auto positions = client.getDronePositions();
                for (const auto& p : positions)
                {
                    operations[p.first] = DRONE_OPERATION_NONE;
                }
                interactionServer->updateDroneOperations(operations);
                interactionServer->updateSwarmState(SWARM_IDLE);
                interactionServer->initialize(12346);
                MSG_INFO("Initializing geometry...");
                geometryModule = new GeometryModule(1.0, 0.7, Position(0.6, 0.6, 0.6, 0.0), Position(0.2, 0.21, 0.4, 0.0), client.getDronePositions());
                MSG_INFO("Intializing swarm operation handler...");
                swarmOperationHandler = new SwarmOperationHandler(*interactionServer, client, *geometryModule);
                MSG_INFO("Running. The system is operational.");
                init = true;
            }
            if (getTimedif(lastPrintTime, getMillis()) >= 1000)
            {
                printDroneSwarmInterfaceClientInformation(client);
                lastPrintTime = getMillis();
            }
        }
        break;
        case DroneSwarmInterfaceClient::DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED:
        {
            MSG_INFO("Client disconnected");
            if (init)
            {
                delete interactionServer;
                delete geometryModule;
                delete swarmOperationHandler;
            }
            std::cout << "Exited gracefully" << std::endl;
            std::exit(0);
        }
        break;
        }

        if (init)
        {
            swarmOperationHandler->update();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}