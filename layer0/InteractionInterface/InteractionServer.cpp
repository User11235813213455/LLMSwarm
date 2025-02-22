#include "InteractionServer.hpp"
#include <optional>
#include <thread>
#include "logger.hpp"

InteractionServer::InteractionServer()
: tcpServer(nullptr), dronePositions(), droneTargets(), droneStates(), droneOperations(), swarmState(SWARM_IDLE), requestQueue(), dataMutex()
{

}
void InteractionServer::initialize(unsigned int pPort, std::string pIP)
{
    if(this->tcpServer == nullptr)
    {
        this->tcpServer = new Server(pPort, [](sockaddr_in){ return true; }, std::bind(&InteractionServer::mainCommunicationThread, this, std::placeholders::_1, std::placeholders::_2), pIP);
    }
}
bool InteractionServer::mainCommunicationThread(sockaddr_in pAddr, int pSocket)
{
    std::optional<std::thread> notificationThread;
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
            exit(0);
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
                if(!notificationThread.has_value())
                {
                    int socket = pSocket;
                    uint32_t interval = notificationRequest.getInterval();
                    notificationThread = std::thread(&InteractionServer::notificationThread, this, socket, interval);
                }
                else
                {
                    /*Ignore on purpose*/
                }
            }
            break;
            case ProtocolMessageID::SET_DRONE_TARGET_POSITION_REQUEST:
            {
                std::string info = "Received SetDroneTargetPositionRequest: ";
                SetDroneTargetPositionsRequest request(data);
                this->dataMutex.lock();
                this->droneTargets = request.getTargets();
                for(const auto& t : this->droneTargets)
                {
                    info += std::to_string(t.first) + ": "+ (std::string)t.second + " ";
                }
                info += "\n";
                this->dataMutex.unlock();
                MSG_INFO(info);
            }
            break;
            case ProtocolMessageID::SWARM_OPERATIONS_REQUEST:
            {
                SwarmOperationRequest request(data);
                this->dataMutex.lock();
                this->requestQueue.push_front(request.getOperation());
                this->dataMutex.unlock();
            }
            break;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}
void InteractionServer::notificationThread(int pSocket, uint32_t pInterval)
{
    while(true)
    {
        this->dataMutex.lock();
        std::map<uint16_t, Position> cpyPos = this->dronePositions;
        std::map<uint16_t, Position> cpyTarget = this->droneTargets;
        std::map<uint16_t, DroneState> cpyStates = this->droneStates;
        std::map<uint16_t, DroneOperation> cpyOperations = this->droneOperations;
        SwarmState cpySwarmState = this->swarmState;
        this->dataMutex.unlock();

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
        delete buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(pInterval));
    }
}
void InteractionServer::updateDronePositions(const std::map<uint16_t, Position>& pPosition)
{
    this->dataMutex.lock();
    this->dronePositions = pPosition;
    this->dataMutex.unlock();
}
void InteractionServer::updateDroneStates(const std::map<uint16_t, DroneState>& pDroneStates)
{
    this->dataMutex.lock();
    this->droneStates = pDroneStates;
    this->dataMutex.unlock();
}
void InteractionServer::updateDroneOperations(const std::map<uint16_t, DroneOperation>& pDroneOperations)
{
    this->dataMutex.lock();
    this->droneOperations = pDroneOperations;
    this->dataMutex.unlock();
}
void InteractionServer::updateSwarmState(SwarmState pSwarmState)
{
    this->dataMutex.lock();
    this->swarmState = pSwarmState;
    this->dataMutex.unlock();
}
std::map<uint16_t, Position> InteractionServer::getTargets()
{
    this->dataMutex.lock();
    std::map<uint16_t, Position> result = this->droneTargets;
    this->dataMutex.unlock();
    return result;
}
std::optional<SwarmOperation> InteractionServer::peekRequest()
{
    SwarmOperation operation;
    this->dataMutex.lock();
    if(this->requestQueue.empty())
    {
        this->dataMutex.unlock();
        return {};
    }
    else
    {
        operation = this->requestQueue.front();
        this->dataMutex.unlock();
        return operation;
    }
}
void InteractionServer::dequeueRequest()
{
    this->dataMutex.lock();
    if(!this->requestQueue.empty())
    {
        this->requestQueue.pop_front();
    }
    
    this->dataMutex.unlock();
}