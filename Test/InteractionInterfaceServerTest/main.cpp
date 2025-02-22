#include "InteractionInterfaceTestClient.hpp"
#include "layer0/InteractionInterface/InteractionServer.hpp"
#include "logger.hpp"

int main()
{
    InteractionServer server;
    server.initialize(12346);

    InteractionInterfaceTestClient client;
    client.initialize(12346, "127.0.0.1");

    MSG_INFO("Waiting for initialization of the client...");
    while(client.getClientState() != InteractionInterfaceTestClient::ClientState::INTERACTION_INTERFACE_TEST_CLIENT_INITIALIZED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    MSG_INFO("Client initialized. Waiting for notification...");
    while(client.getClientState() != InteractionInterfaceTestClient::ClientState::INTERACTION_INTERFACE_TEST_CLIENT_RUNNING)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    client.requestOperation(SwarmOperation::SWARM_OPERATION_TAKEOFF);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(server.peekRequest().has_value())
    {
        if(server.peekRequest().value() == SwarmOperation::SWARM_OPERATION_TAKEOFF)
        {
            MSG_INFO("Operation request check OK.");
        }
        else
        {
            MSG_ERROR("Operation request check failed: Wrong content!");
        }
        server.dequeueRequest();
    }
    else
    {
        MSG_ERROR("Operation request check failed: Request not sent/received!");
    }

    std::map<uint16_t, Position> targets =  {
        {0, Position(1.0, 2.0, 3.0, 4.0)},
        {1, Position(5.0, 6.0, 7.0, 8.0)},
        {2, Position(9.0, 1.0, 2.0, 3.0)}
    };
    client.updateTargets(targets);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(server.getTargets() == targets)
    {
        MSG_INFO("Targets requested successfully");
    }
    else
    {
        MSG_ERROR("Error in updating targets!");
    }

    std::map<uint16_t, Position> positions =  {
        {0, Position(3.0, 5.0, 5.0, 6.0)},
        {1, Position(4.0, 6.0, 4.0, 7.0)},
        {2, Position(1.0, 2.0, 3.0, 4.0)}
    };

    server.updateDronePositions(positions);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if(client.getPositions() == positions)
    {
        MSG_INFO("Positions updated successfully");
    }
    else
    {
        MSG_ERROR("Position update failed!");
    }

    std::map<uint16_t, DroneOperation> operations = {
        {0, DRONE_OPERATION_FAST_STOP},
        {1, DRONE_OPERATION_LAND},
        {2, DRONE_OPERATION_MOVE}
    };

    server.updateDroneOperations(operations);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if(client.getDroneOperations() == operations)
    {
        MSG_INFO("Drone operations update successful");
    }
    else
    {
        MSG_ERROR("Drone operations update failed!");
    }

    std::map<uint16_t, DroneState> states = {
        {0, DRONE_HOVERING},
        {1, DRONE_LANDING},
        {2, DRONE_MOVING}
    };

    server.updateDroneStates(states);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if(client.getDroneStates() == states)
    {
        MSG_INFO("Drone states update successful");
    }
    else
    {
        MSG_ERROR("Drone states update failed!");
    }

    server.updateSwarmState(SWARM_HOVERING);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if(client.getSwarmState() == SWARM_HOVERING)
    {
        MSG_INFO("Swarm state update successful");
    }
    else
    {
        MSG_ERROR("Swarm state update failed!");
    }
}