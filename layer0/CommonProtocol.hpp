#ifndef COMMON_PROTOCOL_HPP_INCLUDED
#define COMMON_PROTOCOL_HPP_INCLUDED

#include <map>
#include "network/protocol.hpp"
#include "position.hpp"

enum ProtocolMessageID
{
    /**
     * @brief Requests cyclic notifications about the current swarm state (drone positions, drone targets, drone states, drone operations, swarm state)
     */
    REGISTER_SWARM_NOTIFICATION_REQUEST=1,

    /**
     * @brief Response to a REGISTER_SWARM_NOTIFICATION_REQUEST to confirm the reception
     */
    REGISTER_SWARM_NOTIFICATION_RESPONSE,

    /**
     * @brief A notification message containing information about the current swarm state (drone positions, drone targets, drone states, drone operations, swarm state)
     */
    SWARM_STATE_NOTIFICATION_MESSAGE,

    /**
     * @brief A request to change the drone target coordinates
     */
    SET_DRONE_TARGET_POSITION_REQUEST,

    /**
     * @brief A response to a SET_DRONE_TARGET_POSITION_REQUEST to confirm reception
     * 
     */
    SET_DRONE_TARGET_POSITION_RESPONSE,

    /**
     * @brief A request to change the swarm operation (e.g. in order to start takeoff, landing, fast stop, movement)
     * 
     */
    SWARM_OPERATIONS_REQUEST,

    /**
     * @brief A response to a SWARM_OPERATIONS_REQUEST to confirm the reception
     * 
     */
    SWARM_OPERATIONS_RESPONSE,

    /**
     * @brief A request to change the operation of the drones
     * 
     */
    DRONE_OPERATIONS_REQUEST,

    /**
     * @brief A response to a DRONE_OPERATIONS_REQUEST confirming the reception
     * 
     */
    DRONE_OPERATIONS_RESPONSE
};

enum DroneOperation
{
    /**
     * @brief Singnals that the drone shall not start any operation currently 
     * 
     */
    DRONE_OPERATION_NONE=0,

    /**
     * @brief Take off operation
     * 
     */
    DRONE_OPERATION_TAKE_OFF=1,

    /**
     * @brief Landing operation
     * 
     */
    DRONE_OPERATION_LAND=2,

    /**
     * @brief "Fast stop" -> Stop immediately and fall to the ground
     * 
     */
    DRONE_OPERATION_FAST_STOP=3,

    /**
     * @brief Move operation -> Move the drone the its target coordinates
     * 
     */
    DRONE_OPERATION_MOVE=4,
};
enum SwarmState
{
    /**
     * @brief Swarm is idle at the floor
     */
    SWARM_IDLE=0,

    /**
     * @brief Swarm is currently taking off
     */
    SWARM_TAKING_OFF,

    /**
     * @brief Swarm is hovering
     */
    SWARM_HOVERING,

    /**
     * @brief Swarm is moving to another position
     */
    SWARM_MOVING,

    /**
     * @brief Swarm is landing
     */
    SWARM_LANDING,

    /**
     * @brief Swarm is fast stopping (will turn off and drop to the floor)
     */
    SWARM_STOPPING
};
enum DroneState
{
    /**
     * @brief The drone is idle on the floor
     */
    DRONE_IDLE=0,

    /**
     * @brief The drone is currently taking off
     */
    DRONE_TAKING_OFF,

    /**
     * @brief The drone is currently hovering in the air without moving
     */
    DRONE_HOVERING,

    /**
     * @brief The drone is currently moving to its target position
     */
    DRONE_MOVING,

    /**
     * @brief The drone is currently landing
     */
    DRONE_LANDING,

    /**
     * @brief The drone is currently stopping
     */
    DRONE_STOPPING
};
enum SwarmOperation
{
    /**
     * @brief Takeoff operation of the swarm; All drones take off and fly to their target coordinates
     */
    SWARM_OPERATION_TAKEOFF=0,

    /**
     * @brief Landing operation of the swarm; All drones land parallel
     */
    SWARM_OPERATION_LAND,

    /**
     * @brief Starts a swarm movement; All drones start moving to their target coordinates
     */
    SWARM_OPERATION_MOVE,

    /**
     * @brief Stops the swarm and lets the drones fall down
     * 
     */
    SWARM_OPERATION_FAST_STOP
};

class RegisterSwarmStateNotificationRequest : public Message
{
public:
    /**
     * @brief Constructs a new RegisterSwarmNotificationRequest 
     * 
     * @param pInterval The interval in ms in which SwarmStateNotifications shall be sent by the receiver of this message
     */
    RegisterSwarmStateNotificationRequest(uint16_t pInterval);

    /**
     * @brief Constructs a RegisterSwarmStateNotificationRequest by serialized data
     * 
     * @param pSerializedData 
     */
    RegisterSwarmStateNotificationRequest(const std::vector<uint8_t>& pSerializedData);

    std::vector<uint8_t> getContent() const override;
    uint32_t parse(const std::vector<uint8_t>& pContent) override;
    uint8_t getID() const override;
    
    /**
     * @brief Returns the interval in ms in which the receiver of this message is supposed to send SwarmStateNotificationRequests
     * 
     * @return uint16_t The interval in ms in which SwarmStateNotifications shall be sent by the receiver of this message
     */
    uint16_t getInterval() const;

    /**
     * @brief Sets the interval in ms in which the receiver of this message is supposed to send SwarmStateNotificationRequests
     * 
     * @param pInterval The interval in ms in which SwarmStateNotifications shall be sent by the receiver of this message
     */
    void setInterval(uint16_t pInterval);
protected:
    /**
     * @brief Update interval in ms
     */
    uint16_t interval;
};
class RegisterSwarmStateNotificiationResponse : public Message
{
public:
    RegisterSwarmStateNotificiationResponse() = default;

    std::vector<uint8_t> getContent() const override;
    uint32_t parse(const std::vector<uint8_t>& pContent) override;
    uint8_t getID() const override;
};
class SwarmStateNotificationMessage : public Message
{
public:
    SwarmStateNotificationMessage(const std::map<uint16_t, Position>& pDronePositions, 
        const std::map<uint16_t, Position>& pDroneTargets, 
        const std::map<uint16_t, DroneState>& pDroneStates,
        const std::map<uint16_t, DroneOperation>& pDroneOperations,
        SwarmState pSwarmState);
    SwarmStateNotificationMessage(const std::vector<uint8_t>& pSerializedData);
    
    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;

    const std::map<uint16_t, Position>& getDronePositions() const;
    void setDronePositions(const std::map<uint16_t, Position>& pDronePositions);

    const std::map<uint16_t, Position>& getDroneTargetPositions() const;
    void setDroneTargetPositions(const std::map<uint16_t, Position>& pDroneTargetPositions);

    const std::map<uint16_t, DroneState>& getDroneStates() const;
    void setDroneStates(const std::map<uint16_t, DroneState>& pDroneStates);

    const std::map<uint16_t, DroneOperation>& getDroneOperations() const;
    void setDroneOperations(const std::map<uint16_t, DroneOperation>& pOperations);

    SwarmState getSwarmState() const;
    void setSwarmState(SwarmState pSwarmState);

protected:
    std::map<uint16_t, Position> dronePositions;
    std::map<uint16_t, Position> droneTargets;
    std::map<uint16_t, DroneState> droneStates;
    std::map<uint16_t, DroneOperation> droneOperations;
    SwarmState swarmState;
};
class SetDroneTargetPositionsRequest : public Message
{
public:
    SetDroneTargetPositionsRequest(std::map<uint16_t, Position>& pTargets);
    SetDroneTargetPositionsRequest(const std::vector<uint8_t>& pSerializedData);
    
    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;

    const std::map<uint16_t, Position>& getTargets() const;
    void setTargets(const std::map<uint16_t, Position>& pPositions);
protected:
    std::map<uint16_t, Position> targets;
};
class SetDroneTargetPositionsResponse : public Message
{
public:
    SetDroneTargetPositionsResponse() = default;
    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;
protected:
};
class SwarmOperationRequest : public Message
{
public:
    SwarmOperationRequest(SwarmOperation pOperation);
    SwarmOperationRequest(const std::vector<uint8_t>& pSerializedData);

    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;

    SwarmOperation getOperation() const;
    void setOperation(SwarmOperation pOperation);
protected:
    SwarmOperation operation;
};
class SwarmOperationResponse : public Message
{
public:
    SwarmOperationResponse() = default;
    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;
protected:
};
class DroneOperationsRequest : public Message
{
public:
    DroneOperationsRequest(const std::map<uint16_t, DroneOperation>& pOperations);
    DroneOperationsRequest(const std::vector<uint8_t>& pSerializedData);

    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;

    const std::map<uint16_t, DroneOperation>& getOperations() const;
    void setOperations(const std::map<uint16_t, DroneOperation>& pOperations);
protected:
    std::map<uint16_t, DroneOperation> operations;
};
class DroneOperationsResponse : public Message
{
public:
    DroneOperationsResponse();
    DroneOperationsResponse(const std::vector<uint8_t>& pSerializedData);

    std::vector<uint8_t> getContent() const;
    uint32_t parse(const std::vector<uint8_t>& pContent);
    uint8_t getID() const;
};

#endif /*COMMON_PROTOCOL_HPP_INCLUDED*/