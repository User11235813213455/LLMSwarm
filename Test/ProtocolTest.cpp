#include "layer0/CommonProtocol.hpp"
#include "logger.hpp"

int main()
{
    std::map<uint16_t, Position> positions{
        {0, Position(0.0, 1.0, 2.0, 3.0)},
        {1, Position(4.0, 5.0, 6.0, 7.0)},
        {2, Position(8.0, 9.0, 10.0, 11.0)},
        {3, Position(12.0, 13.0, 14.0, 15.0)},
        {4, Position(16.0, 17.0, 18.0, 19.0)},
    };
    std::map<uint16_t, Position> targets{
        {0, Position(20.0, 21.0, 22.0, 23.0)},
        {1, Position(24.0, 25.0, 26.0, 27.0)},
        {2, Position(28.0, 29.0, 30.0, 31.0)},
        {3, Position(32.0, 33.0, 34.0, 35.0)},
        {4, Position(36.0, 37.0, 38.0, 39.0)},
    };
    std::map<uint16_t, DroneState> droneStates{
        {0, DRONE_IDLE},
        {1, DRONE_HOVERING},
        {2, DRONE_LANDING},
        {3, DRONE_MOVING},
        {4, DRONE_STOPPING},
    };
    std::map<uint16_t, DroneOperation> droneOperations{
        {0, DroneOperation::DRONE_OPERATION_FAST_STOP},
        {1, DroneOperation::DRONE_OPERATION_LAND},
        {2, DroneOperation::DRONE_OPERATION_MOVE},
        {3, DroneOperation::DRONE_OPERATION_TAKE_OFF},
        {4, DroneOperation::DRONE_OPERATION_LAND},
    };

    SwarmStateNotificationMessage noStates(positions, targets, {}, droneOperations, SWARM_HOVERING);
    SwarmStateNotificationMessage cpy(noStates.serialize());

    std::map<uint16_t, DroneOperation> cpyOperations = cpy.getDroneOperations();
    std::map<uint16_t, Position> cpyPositions = cpy.getDronePositions();
    std::map<uint16_t, Position> cpyTargets = cpy.getDroneTargetPositions();
    std::map<uint16_t, DroneState> cpyStates = cpy.getDroneStates();

    if(cpyPositions == positions && targets == cpyTargets && cpyStates.empty() /*&& cpyOperations == droneOperations*/ && cpy.getSwarmState() == SWARM_HOVERING)
    {
        MSG_INFO("Encoding and decoding of SwarmStateNotificationMessage Check 1 OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in SwarmStateNotificationMessage!");
    }

    SwarmStateNotificationMessage message(positions, targets, droneStates, droneOperations, SWARM_LANDING);
    cpy.parse(message.serialize());

    cpyOperations = cpy.getDroneOperations();
    cpyPositions = cpy.getDronePositions();
    cpyTargets = cpy.getDroneTargetPositions();
    cpyStates = cpy.getDroneStates();
    if(cpyPositions == positions && targets == cpyTargets && cpyStates == droneStates && cpyOperations == droneOperations && cpy.getSwarmState() == SWARM_LANDING)
    {
        MSG_INFO("Encoding and decoding of SwarmStateNotificationMessage Check 2 OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in SwarmStateNotificationMessage!");
    }

    if(RegisterSwarmStateNotificationRequest(RegisterSwarmStateNotificationRequest(1234).serialize()).getInterval() == 1234)
    {
        MSG_INFO("Encoding and decoding of RegisterSwarmStateNotificationRequest OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in RegisterSwarmStateNotificationRequest!");
    }

    if(SetDroneTargetPositionsRequest(SetDroneTargetPositionsRequest(targets).serialize()).getTargets() == targets)
    {
        MSG_INFO("Encoding and decoding of SetDroneTargetPositionsRequest OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in SetDroneTargetPositionsRequest!");
    }

    if(SwarmOperationRequest(SwarmOperationRequest(SWARM_OPERATION_FAST_STOP).serialize()).getOperation() == SWARM_OPERATION_FAST_STOP)
    {
        MSG_INFO("Encoding and decoding of SwarmOperationRequest OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in SwarmOperationRequest!");
    }

    if(DroneOperationsRequest(DroneOperationsRequest(droneOperations).serialize()).getOperations() == droneOperations)
    {
        MSG_INFO("Encoding and decoding of DroneSwarmInterfaceProtocol::DroneOperationsRequest OK");
    }
    else
    {
        MSG_ERROR("Encoding or decoding error in DroneSwarmInterfaceProtocol::DroneOperationsRequest!");
    }
}