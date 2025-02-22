#include "CommonProtocol.hpp"
#include <cstdint>
#include <stdexcept>

RegisterSwarmStateNotificationRequest::RegisterSwarmStateNotificationRequest(uint16_t pInterval)
: interval(pInterval)
{

}
RegisterSwarmStateNotificationRequest::RegisterSwarmStateNotificationRequest(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}
std::vector<uint8_t> RegisterSwarmStateNotificationRequest::getContent() const
{
    std::vector<uint8_t> result;
    uint16_t cpy = this->interval;
    appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&cpy), 2);
    return result;
}
uint32_t RegisterSwarmStateNotificationRequest::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as RegisterSwarmStateNotificationRequest although the ID does not match!"));
    }
    this->interval = networkByteOrderDataToUint16(pContent.begin() + 1);
    return 3;
}
uint8_t RegisterSwarmStateNotificationRequest::getID() const
{
    return ProtocolMessageID::REGISTER_SWARM_NOTIFICATION_REQUEST;
}

uint16_t RegisterSwarmStateNotificationRequest::getInterval() const
{
    return this->interval;
}
void RegisterSwarmStateNotificationRequest::setInterval(uint16_t pInterval)
{
    this->interval = pInterval;
}


std::vector<uint8_t> RegisterSwarmStateNotificiationResponse::getContent() const
{
    return std::vector<uint8_t>();
}
uint32_t RegisterSwarmStateNotificiationResponse::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as RegisterSwarmStateNotificiationResponse although the ID does not match!"));
    }
    return 1;
}
uint8_t RegisterSwarmStateNotificiationResponse::getID() const
{
    return ProtocolMessageID::REGISTER_SWARM_NOTIFICATION_RESPONSE;
}


SwarmStateNotificationMessage::SwarmStateNotificationMessage(const std::map<uint16_t, Position>& pDronePositions, 
    const std::map<uint16_t, Position>& pDroneTargets, 
    const std::map<uint16_t, DroneState>& pDroneStates,
    const std::map<uint16_t, DroneOperation>& pDroneOperations,
    SwarmState pSwarmState)
: dronePositions(pDronePositions), droneTargets(pDroneTargets), droneStates(pDroneStates), droneOperations(pDroneOperations), swarmState(pSwarmState)
{
    
}
SwarmStateNotificationMessage::SwarmStateNotificationMessage(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}

std::vector<uint8_t> SwarmStateNotificationMessage::getContent() const
{
    std::vector<uint8_t> result;
    result.push_back(dronePositions.size());
    for(const auto& p : this->dronePositions)
    {
        uint16_t drone = p.first;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        Position pos = p.second;
        double xD = pos.getX();
        double yD = pos.getY();
        double zD = pos.getZ();
        double yawD = pos.getYaw();

        long x = static_cast<long>(xD * 10000);
        long y = static_cast<long>(yD * 10000);
        long z = static_cast<long>(zD * 10000);
        long yaw = static_cast<long>(yawD * 10000);

        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&x), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&y), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&z), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&yaw), 4);
    }
    result.push_back(this->droneTargets.size());
    for(const auto& p : this->droneTargets)
    {
        uint16_t drone = p.first;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        Position pos = p.second;
        double xD = pos.getX();
        double yD = pos.getY();
        double zD = pos.getZ();
        double yawD = pos.getYaw();

        int32_t x = static_cast<int32_t>(xD * 10000);
        int32_t y = static_cast<int32_t>(yD * 10000);
        int32_t z = static_cast<int32_t>(zD * 10000);
        int32_t yaw = static_cast<int32_t>(yawD * 10000);

        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&x), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&y), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&z), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&yaw), 4);
    }
    result.push_back(this->droneStates.size());
    for(const auto& p : this->droneStates)
    {
        uint16_t drone = p.first;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        DroneState droneState = p.second;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&droneState), 1);
    }
    result.push_back(this->droneOperations.size());
    for(const auto& p : this->droneOperations)
    {
        uint16_t drone = p.first;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        DroneOperation droneOperation = p.second;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&droneOperation), 1);
    }
    result.push_back(this->swarmState);
    return result;
}
uint32_t SwarmStateNotificationMessage::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SwarmStateNotificationMessage although the ID does not match!"));
    }
    this->dronePositions.clear();
    unsigned int numDronePositions = pContent.at(1);
    unsigned int positionsOffset = 2;
    unsigned int cntr = 0;
    for(cntr=0; cntr<numDronePositions; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (positionsOffset + cntr * 18));
        int32_t x = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (positionsOffset + 2 + cntr * 18)));
        int32_t y = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (positionsOffset + 6 + cntr * 18)));
        int32_t z = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (positionsOffset + 10 + cntr * 18)));
        int32_t yaw = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (positionsOffset + 14 + cntr * 18)));

        double xD = x / 10000.0;
        double yD = y / 10000.0;
        double zD = z / 10000.0;
        double yawD = yaw / 10000.0;

        this->dronePositions.insert(std::make_pair(drone, Position(xD, yD, zD, yawD)));
    }
    unsigned int numTargetPositions = pContent.at(numDronePositions * 18 + 2);
    unsigned int targetsOffset = positionsOffset + numDronePositions * 18 + 1;
    for(cntr=0; cntr<numTargetPositions; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (targetsOffset + cntr * 18));
        int32_t x = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 2 + cntr * 18)));
        int32_t y = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 6 + cntr * 18)));
        int32_t z = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 10 + cntr * 18)));
        int32_t yaw = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 14 + cntr * 18)));

        double xD = x / 10000.0;
        double yD = y / 10000.0;
        double zD = z / 10000.0;
        double yawD = yaw / 10000.0;

        this->droneTargets.insert(std::make_pair(drone, Position(xD, yD, zD, yawD)));
    }
    unsigned int numStates = pContent.at(targetsOffset + numTargetPositions * 18);
    unsigned int statesOffset = targetsOffset + numTargetPositions * 18 + 1;
    for(cntr=0; cntr<numStates; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (statesOffset + cntr * 3));
        DroneState state = static_cast<DroneState>(networkByteOrderDataToUint8(pContent.begin() + (statesOffset + 2 + cntr * 3)));

        this->droneStates.insert(std::make_pair(drone, state));
    }
    unsigned int numOperations = pContent.at(statesOffset + numStates * 3);
    unsigned int operationsOffset = statesOffset + numStates * 3 + 1;
    for(cntr=0; cntr<numOperations; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (operationsOffset + cntr * 3));
        DroneOperation operation = static_cast<DroneOperation>(networkByteOrderDataToUint8(pContent.begin() + (operationsOffset + 2 + cntr * 3)));

        this->droneOperations.insert(std::make_pair(drone, operation));
    }

    this->swarmState = static_cast<SwarmState>(networkByteOrderDataToUint8(pContent.begin() + (operationsOffset + numOperations * 3)));
    return operationsOffset + numOperations * 3 + 1;
}
uint8_t SwarmStateNotificationMessage::getID() const
{
    return ProtocolMessageID::SWARM_STATE_NOTIFICATION_MESSAGE;
}

const std::map<uint16_t, Position>& SwarmStateNotificationMessage::getDronePositions() const
{
    return this->dronePositions;
}
void SwarmStateNotificationMessage::setDronePositions(const std::map<uint16_t, Position>& pDronePositions)
{
    this->dronePositions = pDronePositions;
}

const std::map<uint16_t, Position>& SwarmStateNotificationMessage::getDroneTargetPositions() const
{
    return this->droneTargets;
}
void SwarmStateNotificationMessage::setDroneTargetPositions(const std::map<uint16_t, Position>& pDroneTargetPositions)
{
    this->droneTargets = pDroneTargetPositions;
}

const std::map<uint16_t, DroneState>& SwarmStateNotificationMessage::getDroneStates() const
{
    return this->droneStates;
}
void SwarmStateNotificationMessage::setDroneStates(const std::map<uint16_t, DroneState>& pDroneStates)
{
    this->droneStates = pDroneStates;
}

const std::map<uint16_t, DroneOperation>& SwarmStateNotificationMessage::getDroneOperations() const
{
    return this->droneOperations;
}
void SwarmStateNotificationMessage::setDroneOperations(const std::map<uint16_t, DroneOperation>& pOperations)
{
    this->droneOperations = pOperations;
}

SwarmState SwarmStateNotificationMessage::getSwarmState() const
{
    return this->swarmState;
}
void SwarmStateNotificationMessage::setSwarmState(SwarmState pSwarmState)
{
    this->swarmState = pSwarmState;
}

SetDroneTargetPositionsRequest::SetDroneTargetPositionsRequest(std::map<uint16_t, Position>& pTargets)
: targets(pTargets)
{

}
SetDroneTargetPositionsRequest::SetDroneTargetPositionsRequest(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}

std::vector<uint8_t> SetDroneTargetPositionsRequest::getContent() const
{
    std::vector<uint8_t> result;
    result.push_back(this->targets.size());
    for(const auto& p : this->targets)
    {
        uint16_t drone = p.first;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        Position pos = p.second;
        double xD = pos.getX();
        double yD = pos.getY();
        double zD = pos.getZ();
        double yawD = pos.getYaw();

        long x = static_cast<long>(xD * 10000);
        long y = static_cast<long>(yD * 10000);
        long z = static_cast<long>(zD * 10000);
        long yaw = static_cast<long>(yawD * 10000);

        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&x), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&y), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&z), 4);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&yaw), 4);
    }
    return result;
}
uint32_t SetDroneTargetPositionsRequest::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SetDroneTargetPositionsRequest although the ID does not match!"));
    }
    this->targets.clear();
    unsigned int numTargets = pContent.at(1);
    unsigned int targetsOffset = 2;
    unsigned int cntr = 0;
    for(cntr=0; cntr<numTargets; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (targetsOffset + cntr * 18));
        int32_t x = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 2 + cntr * 18)));
        int32_t y = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 6 + cntr * 18)));
        int32_t z = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 10 + cntr * 18)));
        int32_t yaw = static_cast<int32_t>(networkByteOrderDataToUint32(pContent.begin() + (targetsOffset + 14 + cntr * 18)));

        double xD = x / 10000.0;
        double yD = y / 10000.0;
        double zD = z / 10000.0;
        double yawD = yaw / 10000.0;

        this->targets.insert(std::make_pair(drone, Position(xD, yD, zD, yawD)));
    }

    return targetsOffset + numTargets * 18;
}
uint8_t SetDroneTargetPositionsRequest::getID() const
{
    return ProtocolMessageID::SET_DRONE_TARGET_POSITION_REQUEST;
}

const std::map<uint16_t, Position>& SetDroneTargetPositionsRequest::getTargets() const
{
    return this->targets;
}
void SetDroneTargetPositionsRequest::setTargets(const std::map<uint16_t, Position>& pPositions)
{
    this->targets = pPositions;
}


std::vector<uint8_t> SetDroneTargetPositionsResponse::getContent() const
{
    return std::vector<uint8_t>();
}
uint32_t SetDroneTargetPositionsResponse::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SetDroneTargetPositionsResponse although the ID does not match!"));
    }

    return 1;
}
uint8_t SetDroneTargetPositionsResponse::getID() const
{
    return ProtocolMessageID::SET_DRONE_TARGET_POSITION_RESPONSE;
}


SwarmOperationRequest::SwarmOperationRequest(SwarmOperation pOperation)
: operation(pOperation)
{

}
SwarmOperationRequest::SwarmOperationRequest(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}

std::vector<uint8_t> SwarmOperationRequest::getContent() const
{
    std::vector<uint8_t> result;
    result.push_back(static_cast<uint8_t>(operation));
    return result;
}
uint32_t SwarmOperationRequest::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SwarmOperationRequest although the ID does not match!"));
    }
    this->operation = static_cast<SwarmOperation>(networkByteOrderDataToUint8(pContent.begin() + 1));
    return 2;
}
uint8_t SwarmOperationRequest::getID() const
{
    return ProtocolMessageID::SWARM_OPERATIONS_REQUEST;
}

SwarmOperation SwarmOperationRequest::getOperation() const
{
    return this->operation;
}
void SwarmOperationRequest::setOperation(SwarmOperation pOperation)
{
    this->operation = pOperation;
}


std::vector<uint8_t> SwarmOperationResponse::getContent() const
{
    return std::vector<uint8_t>();
}
uint32_t SwarmOperationResponse::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SwarmOperationResponse although the ID does not match!"));
    }
    return 1;
}
uint8_t SwarmOperationResponse::getID() const
{
    return ProtocolMessageID::SWARM_OPERATIONS_RESPONSE;
}
DroneOperationsRequest::DroneOperationsRequest(const std::map<uint16_t, DroneOperation>& pOperations)
{
    this->operations = pOperations;
}
DroneOperationsRequest::DroneOperationsRequest(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}

const std::map<uint16_t, DroneOperation>& DroneOperationsRequest::getOperations() const
{
    return this->operations;
}
void DroneOperationsRequest::setOperations(const std::map<uint16_t, DroneOperation>& pOperations)
{
    this->operations = pOperations;
}

std::vector<uint8_t> DroneOperationsRequest::getContent() const
{
    std::vector<uint8_t> result;
    result.push_back(this->operations.size());
    for(const auto& p : this->operations)
    {
        uint16_t drone = p.first;
        DroneOperation operation = p.second;
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&drone), 2);
        appendNetworkByteOrderInteger(result, reinterpret_cast<uint8_t*>(&operation), 1);
    }
    return result;
}
uint32_t DroneOperationsRequest::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as SetDroneTargetPositionsRequest although the ID does not match!"));
    }
    this->operations.clear();
    unsigned int numTargets = pContent.at(1);
    unsigned int targetsOffset = 2;
    unsigned int cntr = 0;
    for(cntr=0; cntr<numTargets; cntr++)
    {
        uint16_t drone = networkByteOrderDataToUint16(pContent.begin() + (targetsOffset + cntr * 3));
        DroneOperation operation = static_cast<DroneOperation>(networkByteOrderDataToUint8(pContent.begin() + (targetsOffset + 2 + cntr * 3)));
        this->operations.insert(std::make_pair(drone, operation));
    }

    return targetsOffset + numTargets * 3;
}
uint8_t DroneOperationsRequest::getID() const
{
    return ProtocolMessageID::DRONE_OPERATIONS_REQUEST;
}
DroneOperationsResponse::DroneOperationsResponse()
{

}
DroneOperationsResponse::DroneOperationsResponse(const std::vector<uint8_t>& pSerializedData)
{
    this->parse(pSerializedData);
}

std::vector<uint8_t> DroneOperationsResponse::getContent() const
{
    return std::vector<uint8_t>();
}
uint32_t DroneOperationsResponse::parse(const std::vector<uint8_t>& pContent)
{
    if(*pContent.begin() != this->getID())
    {
        throw(std::invalid_argument("Tried to interpret a frame as DroneOperationsResponse although the ID does not match!"));
    }
    return 1;
}
uint8_t DroneOperationsResponse::getID() const
{
    return ProtocolMessageID::DRONE_OPERATIONS_RESPONSE;
}