#include "InteractionInterfaceTestClient.hpp"
#include "logger.hpp"

InteractionInterfaceTestClient::InteractionInterfaceTestClient()
: positions(), targets(), droneStates(), droneOperations(), swarmState(SWARM_IDLE), operation(), dataMutex(), tcpClient(nullptr), state(INTERACTION_INTERFACE_TEST_CLIENT_NOT_INITIALIZED)
{

}

void InteractionInterfaceTestClient::initialize(int pPort, std::string pIP)
{
    if(this->tcpClient == nullptr)
    {
        try
        {
            this->tcpClient = new Client(pPort, std::bind(&InteractionInterfaceTestClient::communicationHandler, this, std::placeholders::_1, std::placeholders::_2), pIP);
            MSG_INFO("Drone swarm interface client successfully initialized");
            this->dataMutex.lock();
            this->state = INTERACTION_INTERFACE_TEST_CLIENT_INITIALIZED;
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

std::map<uint16_t, Position> InteractionInterfaceTestClient::getPositions()
{
    this->dataMutex.lock();
    std::map<uint16_t, Position> cpyPos = this->positions;
    this->dataMutex.unlock();
    return cpyPos;
}
std::map<uint16_t, DroneState> InteractionInterfaceTestClient::getDroneStates()
{
    this->dataMutex.lock();
    std::map<uint16_t, DroneState> cpyStates = this->droneStates;
    this->dataMutex.unlock();
    return cpyStates;
}
std::map<uint16_t, DroneOperation> InteractionInterfaceTestClient::getDroneOperations()
{
    this->dataMutex.lock();
    std::map<uint16_t, DroneOperation> cpyOperations = this->droneOperations;
    this->dataMutex.unlock();
    return cpyOperations;
}
SwarmState InteractionInterfaceTestClient::getSwarmState()
{
    this->dataMutex.lock();
    SwarmState cpySwarmState = this->swarmState;
    this->dataMutex.unlock();
    return cpySwarmState;
}

void InteractionInterfaceTestClient::requestOperation(SwarmOperation pOperation)
{
    this->dataMutex.lock();
    this->operation = pOperation;
    this->dataMutex.unlock();
}
void InteractionInterfaceTestClient::updateTargets(const std::map<uint16_t, Position>& pTargets)
{
    this->dataMutex.lock();
    this->targets = pTargets;
    this->dataMutex.unlock();
}

bool InteractionInterfaceTestClient::communicationHandler(struct sockaddr_in pAddr, int pSocket)
{
    constexpr unsigned int bufferSize = 1000;
    char buffer[bufferSize];
    std::vector<uint8_t> data;

    std::optional<std::map<uint16_t, Position>> lastTargets;
    std::optional<std::map<uint16_t, DroneOperation>> lastOperations;

    while(true)
    {
        this->dataMutex.lock();
        if(this->state == INTERACTION_INTERFACE_TEST_CLIENT_INITIALIZED)
        {
            MSG_INFO("Interaction interface test client entered \"initialized\" state -> send notification request");
            this->dataMutex.unlock();
            break;
        }
        this->dataMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    this->dataMutex.lock();
    this->state = INTERACTION_INTERFACE_TEST_CLIENT_REGISTER_NOTIFICATION;

    /*15 ms update interval*/
    if(sendVector(pSocket, RegisterSwarmStateNotificationRequest(15).serialize()) == -1)
    {
        MSG_ERROR("Could not send register swarm state notification request to server!");
        this->state = INTERACTION_INTERFACE_TEST_CLIENT_DISCONNECTED;
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
                    this->state = INTERACTION_INTERFACE_TEST_CLIENT_RUNNING;
                    this->positions = notification.getDronePositions();
                    this->droneStates = notification.getDroneStates();
                    this->swarmState = notification.getSwarmState();
                    this->droneOperations = notification.getDroneOperations();
                    
                    if(this->targets.has_value() && (!lastTargets.has_value() || lastTargets.value() != this->targets))
                    { 
                        lastTargets = this->targets;
                        if(sendVector(pSocket, SetDroneTargetPositionsRequest(this->targets.value()).serialize()) == -1)
                        {
                            MSG_WARNING("Drone swarm interface client disconnected");
                            this->state = INTERACTION_INTERFACE_TEST_CLIENT_DISCONNECTED;
                            #ifdef linux
                            close(pSocket);
                            #else
                            closesocket(pSocket);
                            #endif
                            this->dataMutex.unlock();
                            return false;
                        }
                    }
                    if(this->operation.has_value())
                    {
                        if(sendVector(pSocket, SwarmOperationRequest(this->operation.value()).serialize()) == -1)
                        {
                            MSG_WARNING("Drone swarm interface client disconnected");
                            this->state = INTERACTION_INTERFACE_TEST_CLIENT_DISCONNECTED;
                            #ifdef linux
                            close(pSocket);
                            #else
                            closesocket(pSocket);
                            #endif
                            this->dataMutex.unlock();
                            return false;
                        }
                        this->operation = {};
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
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
InteractionInterfaceTestClient::ClientState InteractionInterfaceTestClient::getClientState()
{
    this->dataMutex.lock();
    InteractionInterfaceTestClient::ClientState cpyClientState = this->state;
    this->dataMutex.unlock();
    return cpyClientState;
}