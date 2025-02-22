#include "DroneSwarmInterfaceClient.hpp"
#include "logger.hpp"
#include <optional>

DroneSwarmInterfaceClient::DroneSwarmInterfaceClient()
: tcpClient(nullptr), dronePositions(), droneTargets(), droneStates(), swarmState(SWARM_IDLE), droneOperations(), state(DRONE_SWARM_INTERFACE_CLIENT_NOT_INITIALIZED), dataMutex()
{
    MSG_INFO("Drone swarm interface client created");
}

void DroneSwarmInterfaceClient::initialize(unsigned int pPort, std::string pIP)
{
    if(this->tcpClient == nullptr)
    {
        try
        {
            this->tcpClient = new Client(pPort, std::bind(&DroneSwarmInterfaceClient::communicationHandler, this, std::placeholders::_1, std::placeholders::_2), pIP);
            MSG_INFO("Drone swarm interface client successfully initialized");
            this->dataMutex.lock();
            this->state = DRONE_SWARM_INTERFACE_CLIENT_INITIALIZED;
            this->dataMutex.unlock();
        }
        catch(const std::runtime_error& pExc)
        {
            MSG_WARNING(pExc.what());
        }
    }
    else
    {
        /*To prevent a deadlock situation lock the other mutex as well*/
        this->dataMutex.lock();
        MSG_WARNING("Tried to reinitialize the drone swarm interface client!");
        this->dataMutex.unlock();
    }
}

std::map<uint16_t, Position> DroneSwarmInterfaceClient::getDronePositions()
{
    if(this->state != DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        throw(std::runtime_error("Tried to access drone positions of DroneSwarmInterfaceClient although the client is not running currently (or yet)!"));
    }
    std::map<uint16_t, Position> result;
    this->dataMutex.lock();
    result = this->dronePositions;
    this->dataMutex.unlock();
    return result;
}
std::map<uint16_t, DroneState> DroneSwarmInterfaceClient::getDroneStates()
{
    if(this->state != DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        throw(std::runtime_error("Tried to access drone states of DroneSwarmInterfaceClient although the client is not running currently (or yet)!"));
    }
    std::map<uint16_t, DroneState> result;
    this->dataMutex.lock();
    result = this->droneStates;
    this->dataMutex.unlock();
    return result;
}
SwarmState DroneSwarmInterfaceClient::getSwarmState()
{
    if(this->state != DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        throw(std::runtime_error("Tried to access swarm state of DroneSwarmInterfaceClient although the client is not running currently (or yet)!"));
    }
    SwarmState result;
    this->dataMutex.lock();
    result = this->swarmState;
    this->dataMutex.unlock();
    return result;
}

void DroneSwarmInterfaceClient::updateDroneTargets(const std::map<uint16_t, Position>& pTargets)
{
    if(this->state != DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        throw(std::runtime_error("Tried to update drone targets of DroneSwarmInterfaceClient although the client is not running currently (or yet)!"));
    }
    this->dataMutex.lock();
    this->droneTargets = pTargets;
    this->dataMutex.unlock();
}
void DroneSwarmInterfaceClient::updateDroneOperations(const std::map<uint16_t, DroneOperation>& pOperations)
{
    if(this->state != DRONE_SWARM_INTERFACE_CLIENT_RUNNING)
    {
        throw(std::runtime_error("Tried to update drone operations of DroneSwarmInterfaceClient although the client is not running currently (or yet)!"));
    }
    this->dataMutex.lock();
    this->droneOperations = pOperations;
    this->dataMutex.unlock();
}
bool DroneSwarmInterfaceClient::communicationHandler(struct sockaddr_in pAddr, int pSocket)
{
    constexpr unsigned int bufferSize = 2000;
    char buffer[bufferSize];
    std::vector<uint8_t> data;

    std::optional<std::map<uint16_t, Position>> lastTargets;
    std::optional<std::map<uint16_t, DroneOperation>> lastOperations;

    while(true)
    {
        this->dataMutex.lock();
        if(this->state == DRONE_SWARM_INTERFACE_CLIENT_INITIALIZED)
        {
            MSG_INFO("Drone swarm interface client entered \"initialized\" state -> send notification request");
            this->dataMutex.unlock();
            break;
        }
        this->dataMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    this->dataMutex.lock();
    this->state = DRONE_SWARM_INTERFACE_CLIENT_REGISTER_NOTIFICATION;

    /*15 ms update interval*/
    if(sendVector(pSocket, RegisterSwarmStateNotificationRequest(15).serialize()) == -1)
    {
        MSG_ERROR("Could not send register swarm state notification request to server!");
        this->state = DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED;
        #ifdef linux
        close(pSocket);
        #else
        closesocket(pSocket);
        #endif
        this->dataMutex.unlock();
        return false;
    }

    this->dataMutex.unlock();

    while(true)
    {
        data.clear();
        
#ifdef linux
        ssize_t numBytes = recv(pSocket, buffer, bufferSize * sizeof(char), 0);
#else
        int numBytes = recv(pSocket, buffer, bufferSize * sizeof(char), 0);
#endif
        if(numBytes <= 0)
        {
            /*Prevent deadlock*/
            this->dataMutex.lock();
            MSG_WARNING("Drone swarm interface client disconnected");
            this->dataMutex.unlock();
            #ifdef linux
            close(pSocket);
            #else
            closesocket(pSocket);
            #endif
            return false;
        }
        else
        {
            for(unsigned int cntr = 0; cntr < numBytes; cntr++)
            {
                data.push_back(buffer[cntr]);
            }

            switch(data[0])
            {
                case ProtocolMessageID::REGISTER_SWARM_NOTIFICATION_RESPONSE:
                case ProtocolMessageID::SET_DRONE_TARGET_POSITION_RESPONSE:
                case ProtocolMessageID::DRONE_OPERATIONS_RESPONSE:
                {
                    /*TODO: Add a real handling here; Consider timeouts, ...*/
                }
                break;
                case ProtocolMessageID::SWARM_STATE_NOTIFICATION_MESSAGE:
                {
                    SwarmStateNotificationMessage notification(data);
                    this->dataMutex.lock();
                    this->state = DRONE_SWARM_INTERFACE_CLIENT_RUNNING;
                    this->dronePositions = notification.getDronePositions();
                    this->droneStates = notification.getDroneStates();
                    this->swarmState = notification.getSwarmState();
                    
                    if(this->droneTargets.has_value())
                    { 
                        lastTargets = this->droneTargets;
                        if(sendVector(pSocket, SetDroneTargetPositionsRequest(this->droneTargets.value()).serialize()) == -1)
                        {
                            MSG_WARNING("Drone swarm interface client disconnected");
                            this->state = DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED;
                            #ifdef linux
                            close(pSocket);
                            #else
                            closesocket(pSocket);
                            #endif
                            this->dataMutex.unlock();
                            this->droneTargets.reset();
                            return false;
                        }
                        this->droneTargets.reset();
                    }
                    if(this->droneOperations.has_value())
                    {
                        if(sendVector(pSocket, DroneOperationsRequest(this->droneOperations.value()).serialize()) == -1)
                        {
                            MSG_WARNING("Drone swarm interface client disconnected");
                            this->state = DRONE_SWARM_INTERFACE_CLIENT_DISCONNECTED;
                            #ifdef linux
                            close(pSocket);
                            #else
                            closesocket(pSocket);
                            #endif
                            this->dataMutex.unlock();
                            this->droneOperations.reset();
                            return false;
                        }
                        this->droneOperations.reset();
                    }
                    this->dataMutex.unlock();
                }
                break;
                default:
                {
                    /*Ignore all other types of messages intentionally*/
                }
            }
        }
    }
}
DroneSwarmInterfaceClient::ClientState DroneSwarmInterfaceClient::getClientState()
{
    this->dataMutex.lock();
    DroneSwarmInterfaceClient::ClientState clientState = this->state;
    this->dataMutex.unlock();
    return clientState;
}
std::set<uint16_t> DroneSwarmInterfaceClient::getDrones()
{
    std::set<uint16_t> drones;
    std::map<uint16_t, Position> reportedPositions = this->getDronePositions();
    for(const auto& positions : reportedPositions)
    {
        drones.insert(positions.first);
    }
    return drones;
}