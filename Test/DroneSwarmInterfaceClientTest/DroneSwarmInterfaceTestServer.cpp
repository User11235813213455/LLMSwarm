#include "DroneSwarmInterfaceTestServer.hpp"
#include <thread>
#include <optional>
#include "logger.hpp"
#include "utils.hpp"

uint32_t startTime = 0;

DroneSwarmInterfaceTestServer::DroneSwarmInterfaceTestServer(const std::map<uint16_t, Position>& pInitialDronePositions, unsigned short pPort, std::string pIP)
{
    this->initialDronePositions = pInitialDronePositions;
    this->dronePositions = pInitialDronePositions;
    
    for(const auto& d : this->dronePositions)
    {
        this->droneStates.insert(std::make_pair(d.first, DRONE_IDLE));
        this->droneOperationSetpoint.insert(std::make_pair(d.first, DRONE_OPERATION_NONE));
        this->targets.insert(std::make_pair(d.first, d.second));
    }

    this->currentSwarmState = SWARM_IDLE;

    this->server = new Server(pPort, 
        [](sockaddr_in pAddr) { return true; }, 
        std::bind(&DroneSwarmInterfaceTestServer::handleCommunication, this, std::placeholders::_1, std::placeholders::_2), 
        pIP);
}
bool DroneSwarmInterfaceTestServer::handleCommunication(sockaddr_in pAddr, int pSocket)
{
    std::optional<std::thread> nThread;
    std::vector<uint8_t> data;
    char buffer[1000];

    while(true)
    {
        #if linux
        ssize_t numBytes = recv(pSocket, reinterpret_cast<void*>(buffer), 1000, 0);
        #else
        int numBytes = recv(pSocket, buffer, 1000, 0);
        #endif
        if(numBytes <= 0)
        {
            #ifdef linux
            close(pSocket);
            #else
            closesocket(pSocket);
            #endif
            return false;
        }
        
        for(unsigned int cntr = 0; cntr < numBytes; cntr++)
        {
            data.push_back(buffer[cntr]);
        }

        switch(static_cast<ProtocolMessageID>(data[0]))
        {
            case ProtocolMessageID::REGISTER_SWARM_NOTIFICATION_REQUEST:
            {
                RegisterSwarmStateNotificationRequest notificationRequest(data);
                if(!nThread.has_value())
                {
                    int socket = pSocket;
                    uint32_t interval = notificationRequest.getInterval();
                    nThread = std::thread(&DroneSwarmInterfaceTestServer::notificationThread, this, socket, interval);
                }
                else
                {
                    /*Ignore on purpose*/
                }
            }
            break;
            case ProtocolMessageID::SET_DRONE_TARGET_POSITION_REQUEST:
            {
                SetDroneTargetPositionsRequest request(data);
                this->dataMutex.lock();
                this->targets = request.getTargets();
                this->dataMutex.unlock();
            }
            break;
            case ProtocolMessageID::DRONE_OPERATIONS_REQUEST:
            {
                DroneOperationsRequest request(data);
                this->dataMutex.lock();
                this->droneOperationSetpoint = request.getOperations();
                this->dataMutex.unlock();
            }
            break;
            case ProtocolMessageID::DRONE_OPERATIONS_RESPONSE:
            case ProtocolMessageID::SWARM_OPERATIONS_REQUEST:
            case ProtocolMessageID::SWARM_OPERATIONS_RESPONSE:
            case ProtocolMessageID::SET_DRONE_TARGET_POSITION_RESPONSE:
            case ProtocolMessageID::REGISTER_SWARM_NOTIFICATION_RESPONSE:
            {
                /*Ignore on purpose*/
            }
            break;
            default:
            {
                /*Ignore*/
            }
        }

        data.clear();
    }
}
void DroneSwarmInterfaceTestServer::notificationThread(int pSocket, uint32_t pInterval)
{
    while(true)
    {
        this->dataMutex.lock();
        std::map<uint16_t, Position> cpyPos = this->dronePositions;
        std::map<uint16_t, Position> cpyTarget = this->targets;
        std::map<uint16_t, DroneState> cpyStates = this->droneStates;
        std::map<uint16_t, DroneOperation> cpyOperations = this->droneOperationSetpoint;
        SwarmState cpySwarmState = this->currentSwarmState;
        this->dataMutex.unlock();

        for(auto& d : cpyPos)
        {
            d.second.setX(d.second.getX() + (rand() % 100) / 9999.0);
            d.second.setY(d.second.getY() + (rand() % 100) / 9999.0);
            d.second.setZ(d.second.getZ() + (rand() % 100) / 9999.0);
        }

        SwarmStateNotificationMessage message(cpyPos, cpyTarget, cpyStates, cpyOperations, cpySwarmState);
        std::vector<uint8_t> data = message.serialize();
        char* buffer = new char[data.size()];
        unsigned int cntr = 0;
        for(cntr=0; cntr<data.size(); cntr++)
        {
            buffer[cntr] = static_cast<char>(data[cntr]);
        }
        if(send(pSocket, buffer, data.size(), 0) == -1)
        {
            delete buffer;
            return;
        }
        delete[] buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(pInterval));
    }
}
void DroneSwarmInterfaceTestServer::handleTakeoffRequest(uint16_t pDrone)
{
    this->dataMutex.lock();
    DroneState state = this->droneStates.at(pDrone);
    this->dataMutex.unlock();

    switch(state)
    {
        case DRONE_IDLE:
        {
            this->dataMutex.lock();
            this->droneStates.at(pDrone) = DRONE_TAKING_OFF;
            this->dataMutex.unlock();
        }
        break;
        case DRONE_LANDING:
        case DRONE_MOVING:
        case DRONE_STOPPING:
        case DRONE_HOVERING:
        case DRONE_TAKING_OFF:
        {
            /*Ignore*/
        }
        break;
    }
}
void DroneSwarmInterfaceTestServer::handleLandRequest(uint16_t pDrone)
{
    this->dataMutex.lock();
    DroneState state = this->droneStates.at(pDrone);
    this->dataMutex.unlock();

    switch(state)
    {
        case DRONE_HOVERING:
        {
            this->dataMutex.lock();
            this->droneStates.at(pDrone) = DRONE_LANDING;
            this->dataMutex.unlock();
        }
        break;
        case DRONE_LANDING:
        case DRONE_MOVING:
        case DRONE_STOPPING:
        case DRONE_IDLE:
        case DRONE_TAKING_OFF:
        {
            /*Ignore*/
        }
        break;
    }
}
void DroneSwarmInterfaceTestServer::handleFastStopRequest(uint16_t pDrone)
{
    this->dataMutex.lock();
    DroneState state = this->droneStates.at(pDrone);
    this->dataMutex.unlock();

    switch(state)
    {
        case DRONE_IDLE:
        case DRONE_STOPPING:
        {
            /*We are already stopping or stopped*/
        }
        break;
        case DRONE_LANDING:
        case DRONE_MOVING:
        case DRONE_TAKING_OFF:
        case DRONE_HOVERING:
        {
            this->dataMutex.lock();
            this->droneStates.at(pDrone) = DRONE_STOPPING;
            this->dataMutex.unlock();
        }
        break;
    }
}
void DroneSwarmInterfaceTestServer::handleMoveRequest(uint16_t pDrone)
{
    this->dataMutex.lock();
    DroneState state = this->droneStates.at(pDrone);
    this->dataMutex.unlock();

    switch(state)
    {
        case DRONE_HOVERING:
        {
            this->dataMutex.lock();
            this->droneStates.at(pDrone) = DRONE_MOVING;
            startTime = getMillis();
            this->dataMutex.unlock();
        }
        break;
        case DRONE_IDLE:
        case DRONE_STOPPING:
        case DRONE_LANDING:
        case DRONE_MOVING:
        case DRONE_TAKING_OFF:
        {
            /*Ignore on purpose as we either can't or shouldn't move*/
        }
        break;
    }
}
void DroneSwarmInterfaceTestServer::handleTakingOffState(uint16_t pDrone)
{
    this->dataMutex.lock();
    Position currentPosition = this->dronePositions.at(pDrone);
    Position targetPosition = this->targets.at(pDrone);
    this->currentSwarmState = SWARM_TAKING_OFF;
    this->dataMutex.unlock();

    if(currentPosition.getEuclideanDistance(targetPosition) <= 0.011)
    {
        this->dataMutex.lock();
        this->droneStates.at(pDrone) = DRONE_HOVERING;
        this->droneOperationSetpoint.at(pDrone) = DRONE_OPERATION_NONE;
        this->dataMutex.unlock();
    }
    else
    {
        /*Move 0.01 in direction of the target*/
        Position difference = targetPosition - currentPosition;
        Position normalizedDifference = difference * (1.0 / difference.getAbs());
        Position nextPosition = normalizedDifference * 0.0025 + currentPosition;
        this->dataMutex.lock();
        this->dronePositions.at(pDrone) = nextPosition;
        this->dataMutex.unlock();
    }
}
void DroneSwarmInterfaceTestServer::handleLandingState(uint16_t pDrone)
{
    this->dataMutex.lock();
    Position currentPosition = this->dronePositions.at(pDrone);
    this->currentSwarmState = SWARM_LANDING;
    this->dataMutex.unlock();

    Position targetPosition(currentPosition.getX(), currentPosition.getY(), 0.0, currentPosition.getYaw());

    if(currentPosition.getEuclideanDistance(targetPosition) <= 0.011)
    {
        this->dataMutex.lock();
        this->droneStates.at(pDrone) = DRONE_IDLE;
        this->droneOperationSetpoint.at(pDrone) = DRONE_OPERATION_NONE;
        this->dataMutex.unlock();
    }
    else
    {
        /*Move 0.01 in direction of the target*/
        Position difference = targetPosition - currentPosition;
        Position normalizedDifference = difference * (1.0 / difference.getAbs());
        Position nextPosition = normalizedDifference * 0.0025 + currentPosition;
        this->dataMutex.lock();
        this->dronePositions.at(pDrone) = nextPosition;
        this->dataMutex.unlock();
    }
}
void DroneSwarmInterfaceTestServer::handleStoppingState(uint16_t pDrone)
{
    this->dataMutex.lock();
    Position currentPosition = this->dronePositions.at(pDrone);
    this->currentSwarmState = SWARM_STOPPING;
    this->dataMutex.unlock();

    Position targetPosition(currentPosition.getX(), currentPosition.getY(), 0.0, currentPosition.getYaw());

    if(currentPosition.getEuclideanDistance(targetPosition) <= 0.051)
    {
        this->dataMutex.lock();
        this->droneStates.at(pDrone) = DRONE_IDLE;
        this->droneOperationSetpoint.at(pDrone) = DRONE_OPERATION_NONE;
        this->dataMutex.unlock();
    }
    else
    {
        /*Move 0.5 in direction of the target*/
        Position difference = targetPosition - currentPosition;
        Position normalizedDifference = difference * (1.0 / difference.getAbs());
        Position nextPosition = normalizedDifference * 0.04 + currentPosition;
        this->dataMutex.lock();
        this->dronePositions.at(pDrone) = nextPosition;
        this->dataMutex.unlock();
    }
}
void DroneSwarmInterfaceTestServer::handleMovingState(uint16_t pDrone)
{
    this->dataMutex.lock();
    Position currentPosition = this->dronePositions.at(pDrone);
    Position targetPosition = this->targets.at(pDrone);
    this->currentSwarmState = SWARM_MOVING;
    this->dataMutex.unlock();

    if(currentPosition.getEuclideanDistance(targetPosition) <= 0.011 && getTimedif(startTime, getMillis()) >= 100)
    {
        this->dataMutex.lock();
        this->droneStates.at(pDrone) = DRONE_HOVERING;
        this->droneOperationSetpoint.at(pDrone) = DRONE_OPERATION_NONE;
        this->dataMutex.unlock();
    }
    else
    {
        /*Move 0.1 in direction of the target*/
        Position difference = targetPosition - currentPosition;
        Position normalizedDifference = difference * (1.0 / difference.getAbs());
        Position nextPosition = normalizedDifference * 0.0025 + currentPosition;
        this->dataMutex.lock();
        this->dronePositions.at(pDrone) = nextPosition;
        this->dataMutex.unlock();
    }
}
void DroneSwarmInterfaceTestServer::update()
{
    this->dataMutex.lock();
    std::map<uint16_t, DroneOperation> cpyOperations = this->droneOperationSetpoint;
    this->dataMutex.unlock();
    
    for(const auto& drone : cpyOperations)
    {
        switch(drone.second)
        {
            case DRONE_OPERATION_NONE:
            {
                /*Do nothing*/
            }
            break;
            case DRONE_OPERATION_TAKE_OFF:
            {
                this->handleTakeoffRequest(drone.first);
            }
            break;
            case DRONE_OPERATION_LAND:
            {
                this->handleLandRequest(drone.first);
            }
            break;
            case DRONE_OPERATION_MOVE:
            {
                this->handleMoveRequest(drone.first);
            }
            break;
            case DRONE_OPERATION_FAST_STOP:
            {
                this->handleFastStopRequest(drone.first);
            }
            break;
        }
    }

    this->dataMutex.lock();
    std::map<uint16_t, DroneState> cpyStates = this->droneStates;
    this->dataMutex.unlock();

    bool allIdle = true;
    bool allHovering = true;

    for(const auto& drone : cpyStates)
    {
        switch(drone.second)
        {
            case DRONE_HOVERING:
            {
                allIdle = false;
            }
            break;
            case DRONE_IDLE:
            {
                allHovering = false;
            }
            break;
            case DRONE_MOVING:
            {
                allIdle = false;
                allHovering = false;
                this->handleMovingState(drone.first);
            }
            break;
            case DRONE_TAKING_OFF:
            {
                allIdle = false;
                allHovering = false;
                this->handleTakingOffState(drone.first);
            }
            break;
            case DRONE_LANDING:
            {
                allIdle = false;
                allHovering = false;
                this->handleLandingState(drone.first);
            }
            break;
            case DRONE_STOPPING:
            {
                allIdle = false;
                allHovering = false;
                this->handleStoppingState(drone.first);
            }
            break;
        }
    }
    if(allIdle)
    {
        this->dataMutex.lock();
        this->currentSwarmState = SWARM_IDLE;
        this->dataMutex.unlock();
    }
    else if(allHovering)
    {
        this->dataMutex.lock();
        this->currentSwarmState = SWARM_HOVERING;
        this->dataMutex.unlock();
    }
}
void DroneSwarmInterfaceTestServer::print()
{
    this->dataMutex.lock();
    auto cpyOps = this->droneOperationSetpoint;
    auto cpyTargets = this->targets;
    auto cpyPositions = this->dronePositions;
    auto cpySwarmState = this->currentSwarmState;
    auto cpyStates = this->droneStates;
    this->dataMutex.unlock();

    std::string operationsStr;

    for(const auto& d : cpyOps)
    {
        operationsStr += "(";
        operationsStr += std::to_string(d.first);
        operationsStr += ",";
        operationsStr += std::to_string(d.second);
        operationsStr += ") ";
    }

    std::string positionsStr;

    for(const auto& d : cpyPositions)
    {
        positionsStr += "(";
        positionsStr += std::to_string(d.first);
        positionsStr += ",";
        positionsStr += d.second;
        positionsStr += ") ";
    }

    std::string targetsStr;

    for(const auto& d : cpyTargets)
    {
        targetsStr += "(";
        targetsStr += std::to_string(d.first);
        targetsStr += ",";
        targetsStr += d.second;
        targetsStr += ") ";
    }

    std::string statesStr;

    for(const auto& d : cpyStates)
    {
        statesStr += "(";
        statesStr += std::to_string(d.first);
        statesStr += ",";
        statesStr += std::to_string(d.second);
        statesStr += ") ";
    }

    std::string swarmState = std::to_string(cpySwarmState);

    /*MSG_INFO("Operations: " + operationsStr);
    MSG_INFO("States: " + statesStr);
    MSG_INFO("Positions: " + positionsStr);
    MSG_INFO("Targets: " + targetsStr);
    MSG_INFO("Swarm state: " + swarmState);*/
}