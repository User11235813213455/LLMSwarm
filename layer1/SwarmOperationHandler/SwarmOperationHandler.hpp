#ifndef SWARM_OPERATION_HANDLER_HPP_INCLUDED
#define SWARM_OPERATION_HANDLER_HPP_INCLUDED

#include "layer0/InteractionInterface/InteractionServer.hpp"
#include "layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.hpp"
#include "layer0/GeometryModule/GeometryModule.hpp"
#include <optional>

/**
 * @brief Core element of the Operation Controller; This class puts all of the components (InteractionServer, DroneSwarmInterfaceClient, GeometryModule) together and manages their interactions. Requests are received from the interaction server, plan calculations are started based on the information of the Geometry module and the plan is executed using the DroneSwarmInterfaceClient.
 */
class SwarmOperationHandler
{
public:
    /**
     * @brief Creates a new SwarmOperationHandler, which needs an InteractionServer to serve as communication interface to an InteractionClient (e.g. an AR application), a DroneSwarmInterfaceClient which communicates with the drone swarm and a GeometryModule which manages an environment graph in which the drones are flying
     */
    SwarmOperationHandler(InteractionServer& pInteractionServer, DroneSwarmInterfaceClient& pDroneSwarmInterfaceClient, GeometryModule& pGeometry);

    /**
     * @brief Update function which needs to be called cyclically
     */
    void update();

    /**
     * @brief A concrete plan which will be given to the drone swarm step by step
     */
    class Plan
    {
    public:
        /**
         * @brief Represents states of a Plan
         */
        enum State
        {
            /*The plan was not initialized yet*/
            PLAN_INITIALIZE,
            /*Wait for all drones to hover before continuing*/
            PLAN_WAIT_FOR_FIRST_HOVER,
            /*Send the next target coordinates and issue move operations for the drones*/
            PLAN_SEND_NEXT_TARGETS,
            /*Wait until the drones reach proximity to their targets*/
            PLAN_WAIT_FOR_TARGET,
            /*Wait until all drones are hovering*/
            PLAN_WAIT_FOR_HOVER,
            /*The plan is done*/
            PLAN_DONE
        };
        
        /**
         * @brief Copy constructor
         * @param pOther Plan to copy
         */
        Plan(const Plan& pOther);

        /**
         * @brief Creates a plan based on a SwarmOperationHandler and specific steps calculated by the collision avoidance and pathfinding algorithm
         * @param pSwarmOperationHandler The SwarmOperationHandler which will execute this plan
         * @param pSteps The concrete steps for the plan as a vector of mappings agent -> position
         */
        Plan(SwarmOperationHandler& pSwarmOperationHandler, const std::vector<std::map<uint16_t, Position>>& pSteps);

        /**
         * @brief Update function which needs to be called cyclically
         */
        void update();

        /**
         * @brief Returns the current state of the plan
         * @return Current plan state
         */
        State getState() const;

        /**
         * @brief Returns all steps of the plan 
         * @return Vector of mappings agent -> position containing the plan for the drones as const reference
         */
        const std::vector<std::map<uint16_t, Position>>& getSteps() const;

        /**
         * @brief Returns the current step as mapping agent -> position
         * @return Mapping agent -> position for the current step
         */
        std::map<uint16_t, Position> getCurrentStep() const;
    protected:
        /*Stored reference to the corresponding SwarmOperationHandler*/
        SwarmOperationHandler& swarmOperationHandler;

        /*Stores the current state*/
        State currentState;

        /*Stores the steps of the plan*/
        std::vector<std::map<uint16_t, Position>> steps;

        /*Iterator to the current step of the plan*/
        std::vector<std::map<uint16_t, Position>>::iterator currentStep;

        /*Iterator to the previous step*/
        std::vector<std::map<uint16_t, Position>>::iterator lastStep;
    };

protected:
    /*Stores the InteractionServer used by the SwarmOperationHandler*/
    InteractionServer& interactionServer;
    /*The DroneSwarmInterfaceClient to use to control the swarm*/
    DroneSwarmInterfaceClient& droneSwarmInterfaceClient;
    /*The GeometryModule containing the environment information of the swarm*/
    GeometryModule& geometry;

private:
    /**
     * @brief Helper function which is called when a takeoff request is received from the InteractionServer
     */
    void handleTakeoffRequest();

    /**
     * @brief Helper function which is called when a land request is received from the InteractionServer
     */
    void handleLandRequest();

    /**
     * @brief Helper function which is called when a fast stop request is received from the InteractionServer
     */
    void handleFastStopRequest();

    /**
     * @brief Helper function which is called when a move request is received from the Interaction Server
     */
    void handleMoveRequest();

    /**
     * @brief Stores the current plan for the drones (if any)
     */
    std::optional<Plan> plan;
};

#endif /*SWARM_OPERATION_HANDLER_HPP_INCLUDED*/