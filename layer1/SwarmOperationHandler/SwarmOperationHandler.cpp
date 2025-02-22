#include "SwarmOperationHandler.hpp"
#include "graph/MAPF/CBS/CBS.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <vector>

#define PATH_MERGING 0
#define PATH_REFINING 0

void SwarmOperationHandler::handleTakeoffRequest()
{
    MSG_INFO("Handle takeoff request...");
    switch(this->droneSwarmInterfaceClient.getSwarmState())
    {
        case SWARM_IDLE:
        case SWARM_LANDING:
        {
            /*Get snap points close to the drones*/
            std::map<uint16_t, Position> targets = this->droneSwarmInterfaceClient.getDronePositions();

            /*Calculate snap points*/
            std::map<uint16_t, NodeType> snapped = this->geometry.snap<uint16_t>(targets);

            /*Translate back to real world coordinates*/
            std::map<uint16_t, Position> realWorldCoordinates = this->geometry.translateToRealWorldCoordinates(snapped);

            std::string targetsStr = "Snap points to fly to after takeoff: ";
            for(const auto& t : realWorldCoordinates)
            {
                targetsStr += t.second;
                targetsStr += " ";
            }

            MSG_INFO(targetsStr);

            /*Set initial targets for the drones to move to after finishing their takeoff*/
            this->droneSwarmInterfaceClient.updateDroneTargets(realWorldCoordinates);
            
            std::map<uint16_t, DroneOperation> operations;
            for(const auto& drone : targets)
            {
                /*Make all drones take off simultaniously*/
                operations.insert(std::make_pair(drone.first, DRONE_OPERATION_TAKE_OFF));
            }

            this->droneSwarmInterfaceClient.updateDroneOperations(operations);
            MSG_INFO("Started takeoff operation");
        }
        break;
        case SWARM_HOVERING:
        case SWARM_MOVING:
        case SWARM_STOPPING:
        case SWARM_TAKING_OFF:
        {
            /*Ignore*/
            MSG_WARNING("Request ignored due to state!");
        }
        break;
    }
}
void SwarmOperationHandler::handleLandRequest()
{
    MSG_INFO("Handle land request...");
    switch(this->droneSwarmInterfaceClient.getSwarmState())
    {
        case SWARM_IDLE:
        case SWARM_TAKING_OFF:
        case SWARM_HOVERING:
        {   
            std::set<uint16_t> drones = this->droneSwarmInterfaceClient.getDrones();
            std::map<uint16_t, DroneOperation> operations;
            for(const auto& drone : drones)
            {
                /*Make all drones take off simultaniously*/
                operations.insert(std::make_pair(drone, DRONE_OPERATION_LAND));
            }

            this->droneSwarmInterfaceClient.updateDroneOperations(operations);
            MSG_INFO("Started landing operation");
        }
        break;
        case SWARM_MOVING:
        case SWARM_STOPPING:
        case SWARM_LANDING:
        {
            /*Ignore*/
            MSG_WARNING("Request ignored due to state!");
        }
        break;
    }
}
void SwarmOperationHandler::handleFastStopRequest()
{
    MSG_INFO("Handle fast stop request...");
    switch(this->droneSwarmInterfaceClient.getSwarmState())
    {
        case SWARM_IDLE:
        case SWARM_STOPPING:
        {
            MSG_WARNING("Request will probably be ignored due to state!");
        }
        case SWARM_LANDING:
        case SWARM_MOVING:
        case SWARM_HOVERING:
        case SWARM_TAKING_OFF:
        {   
            std::set<uint16_t> drones = this->droneSwarmInterfaceClient.getDrones();
            std::map<uint16_t, DroneOperation> operations;
            for(const auto& drone : drones)
            {
                /*Make all drones stop simultaniously*/
                operations.insert(std::make_pair(drone, DRONE_OPERATION_FAST_STOP));
            }

            this->droneSwarmInterfaceClient.updateDroneOperations(operations);
            MSG_INFO("Started fast stop operation");
        }
        break;
    }
}
void SwarmOperationHandler::handleMoveRequest()
{
    MSG_INFO("Handle move request...");
    switch(this->droneSwarmInterfaceClient.getSwarmState())
    {
        case SWARM_MOVING:
        case SWARM_HOVERING:
        {
            /*Snap target and current coordinates*/
            std::map<uint16_t, Position> targets = this->interactionServer.getTargets();
            std::map<uint16_t, Position> current = this->droneSwarmInterfaceClient.getDronePositions();

            /*Perform a quick check to see if the drones match*/
            if(std::none_of(targets.begin(), targets.end(), [&](const auto& pTarget) {
                return !current.contains(pTarget.first);
            }))
            {
                /*Calculate snap points*/
                std::string originalStr = "Original targets: ";
                for(const auto& t : targets)
                {
                    originalStr += t.second;
                    originalStr += " ";
                }

                MSG_INFO(originalStr);

                std::string originalPosStr = "Original positions: ";
                for (const auto& c : current)
                {
                    originalPosStr += c.second;
                    originalPosStr += " ";
                }

                MSG_INFO(originalPosStr);
                
                std::map<uint16_t, NodeType> snappedTargets = this->geometry.snap<uint16_t>(targets);
                std::map<uint16_t, NodeType> snappedPositions = this->geometry.snap<uint16_t>(current);

                std::map<uint16_t, Position> realWorldCoordinates = this->geometry.translateToRealWorldCoordinates<uint16_t>(snappedTargets);

                bool updatePath = true;

                if (this->plan.has_value())
                {
                    updatePath = false;
                    /*We already have a plan, let's check if the target has changed*/

                    const Plan& p = this->plan.value();
                    if (p.getSteps().size() < 1)
                    {
                        updatePath = true;
                    }
                    else
                    {
                        const std::vector<std::map<uint16_t, Position>>& steps = p.getSteps();
                        const std::map<uint16_t, NodeType> lastSteps = this->geometry.snap<uint16_t>(steps.back());

                        /*Check if the plans matches*/
                        for (const auto& t : realWorldCoordinates)
                        {
                            if (lastSteps.contains(t.first))
                            {
                                if (lastSteps.at(t.first) != snappedTargets.at(t.first))
                                {
                                    updatePath = true;
                                }
                            }
                            else
                            {
                                updatePath = true;
                            }
                        }

                        if (updatePath)
                        {
                            switch (p.getState())
                            {
                                case Plan::PLAN_WAIT_FOR_FIRST_HOVER:
                                case Plan::PLAN_SEND_NEXT_TARGETS:
                                case Plan::PLAN_INITIALIZE:
                                case Plan::PLAN_DONE:
                                {
                                    /*No active target -> Don't update start position of new plan*/
                                }
                                break;
                                case Plan::PLAN_WAIT_FOR_HOVER:
                                case Plan::PLAN_WAIT_FOR_TARGET:
                                {
                                    /*We are already moving -> Assume we start at the next target*/

                                    std::string startMsg = "Due to active movement assume starting points not at ";
                                    for (const auto& c : snappedPositions)
                                    {
                                        startMsg += c.second;
                                        startMsg += " ";
                                    }

                                    startMsg += "but at ";

                                    snappedPositions = this->geometry.snap<uint16_t>(plan.value().getCurrentStep());
                                    for (const auto& c : snappedPositions)
                                    {
                                        startMsg += c.second;
                                        startMsg += " ";
                                    }

                                    MSG_INFO(startMsg);

                                    
                                }
                                break;
                            }
                        }
                    }
                }

                if (updatePath)
                {
                    std::string targetsStr = "Snap points to fly to: ";
                    for (const auto& t : realWorldCoordinates)
                    {
                        targetsStr += t.second;
                        targetsStr += " ";
                    }

                    MSG_INFO(targetsStr);

                    CBS solver = CBS([](NodeType pNode1, NodeType pNode2) {
                        std::tuple<uint32_t, uint32_t, uint32_t> pos1 = GeometryModule::getNodeHypercubePosition(pNode1);
                        std::tuple<uint32_t, uint32_t, uint32_t> pos2 = GeometryModule::getNodeHypercubePosition(pNode2);

                        return Position(std::get<0>(pos1),
                            std::get<1>(pos1), std::get<2>(pos1), 0.0).getEuclideanDistance(Position(std::get<0>(pos2), 
                            std::get<1>(pos2), std::get<2>(pos2), 0.0));
                    });

                    std::map<unsigned int, std::pair<NodeType, NodeType>> agents = {};
                    for (const auto& t : snappedTargets)
                    {
                        NodeType start = snappedPositions.at(t.first);
                        NodeType target = t.second;
                        agents[t.first] = std::make_pair(start, target);
                    }

                    Graph environmentGraph = this->geometry.getEnvironmentGraph();
                    MAPF::Task task(environmentGraph, agents);

                    unsigned int entryTime = getMillis();
                    MAPF::Plan mapfPlan = solver.solveTask(task);
                    MSG_INFO("Calculated path using CBS after " + std::to_string(getTimedif(entryTime, getMillis())) + " ms");

                    std::vector<std::map<unsigned int, NodeType>> nodePlan = {};
                    std::vector<std::map<uint16_t, Position>> planData = {};
                    bool skip = true;

                    mapfPlan.simulate([&](const std::map<unsigned int, NodeType>& pPositions) {
                        std::map<unsigned int, Position> realWorldCoordinates = this->geometry.translateToRealWorldCoordinates<unsigned int>(pPositions);
                        std::map<uint16_t, Position> droneTargets;

                        std::string targetsStr = "Calculated next drone targets as: ";

                        for (const auto& r : realWorldCoordinates)
                        {
                            droneTargets.insert(std::make_pair(r.first, r.second));
                            targetsStr += r.second;
                            targetsStr += " ";
                        }

                        MSG_INFO(targetsStr);

                        planData.push_back(droneTargets);
                        nodePlan.push_back(pPositions);
                    });

#if PATH_MERGING
                    std::vector<std::map<uint16_t, Position>> mergedPlan = planData;
                    /*Path merging*/
                    for (const auto& dronePos : current)
                    {
                        std::vector<Position> disjunctPositions;
                        bool nonMergable = false;

                        unsigned int cntrStep = 0;
                        for (const auto& step : planData)
                        {
                            nonMergable = std::any_of(planData.begin(), planData.end(), [&](const auto& v) {
                                return std::any_of(v.begin(), v.end(), [&](const auto& kvp) {
                                        if (dronePos.first == kvp.first)
                                        {
                                            /*No conflict with itself*/
                                            return false;
                                        }
                                        if (std::find(disjunctPositions.begin(), disjunctPositions.end(), kvp.second) != disjunctPositions.end())
                                        {
                                            /*Another drone uses this node*/
                                            return true;
                                        }
                                        else
                                        {
                                            return false;
                                        }
                                    }
                                );
                            });
                            bool allShareXZ = true;
                            bool allShareYZ = true;
                            bool allShareXY = true;

                            std::optional<Position> lastPosition;

                            for (const auto& pos : disjunctPositions)
                            {
                                if (!lastPosition.has_value())
                                {
                                    lastPosition.emplace(pos);
                                }
                                else
                                {
                                    if (allShareXY)
                                    {
                                        if (lastPosition.value().getX() != pos.getX() || lastPosition.value().getY() != pos.getY())
                                        {
                                            allShareXY = false;
                                        }
                                    }
                                    if (allShareXZ)
                                    {
                                        if (lastPosition.value().getX() != pos.getX() || lastPosition.value().getZ() != pos.getZ())
                                        {
                                            allShareXZ = false;
                                        }
                                    }
                                    if (allShareYZ)
                                    {
                                        if (lastPosition.value().getY() != pos.getY() || lastPosition.value().getZ() != pos.getZ())
                                        {
                                            allShareYZ = false;
                                        }
                                    }

                                    if (!(allShareXY || allShareXZ || allShareYZ))
                                    {
                                        /*Might as well exit early*/
                                        break;
                                    }
                                }
                            }

                            if (!(allShareXY || allShareXZ || allShareYZ))
                            {
                                /*Points are not parallel to axis => Can't merge*/
                                nonMergable = true;
                            }

                            if (nonMergable)
                            {
                                if (disjunctPositions.size() > 3)
                                {
                                    unsigned int cntr = 0;
                                    for (cntr = 1; cntr + 1 < disjunctPositions.size(); cntr++)
                                    {
                                        mergedPlan[cntrStep + cntr - disjunctPositions.size()][dronePos.first] = disjunctPositions.at(disjunctPositions.size() - 2);
                                    }
                                }
                                disjunctPositions.clear();
                            }
                            else
                            {
                                disjunctPositions.push_back(step.at(dronePos.first));
                            }
                            cntrStep++;
                        }
                        if (!nonMergable)
                        {
                            if (disjunctPositions.size() > 3)
                            {
                                unsigned int cntr = 0;
                                for (cntr = 1; cntr + 1 < disjunctPositions.size(); cntr++)
                                {
                                    mergedPlan[cntrStep + cntr - disjunctPositions.size()][dronePos.first] = disjunctPositions.at(disjunctPositions.size() - 2);
                                }
                            }
                            disjunctPositions.clear();
                        }
                        disjunctPositions.clear();
                    }
                    planData = mergedPlan;
#endif
#if PATH_REFINING
                    if (planData.size() > 1)
                    {
                        /*Maps a timestamp to a set of drones which do not move from this point on*/
                        std::map<uint16_t, std::set<uint16_t>> refinementPos;

                        for (const auto& p : current)
                        {
                            Position lastPos = planData.back().at(p.first);
                            unsigned int cntr = planData.size();
                            std::vector<std::map<uint16_t, Position>>::reverse_iterator r;
                            for (r = planData.rbegin(); r != planData.rend(); r++)
                            {
                                if (lastPos == r->at(p.first))
                                {
                                    /*Still the same position*/
                                    cntr--;
                                }
                                else
                                {
                                    /*Change of position -> Found first point where the drone stood still*/
                                    break;
                                }
                            }
                            if (cntr == 0)
                            {
                                /*All the same position -> Refinement at 1 (to force the drones to at least first fly to their nodes)*/
                                cntr = 1;
                            }
                            else if (cntr == planData.size() - 1)
                            {
                                /*Last step -> Insert one refinement step*/
                                planData.push_back(planData.back());
                                nodePlan.push_back(nodePlan.back());
                                cntr = planData.size() - 1;
                            }
                            refinementPos[cntr].insert(p.first);
                        }
                        for (const auto& r : refinementPos)
                        {
                            /*If we want the refinement at time x, we also want it for >x if possible*/
                            for (const auto& d : r.second)
                            {
                                int cntr;
                                for (cntr = r.first; cntr < planData.size(); cntr++)
                                {
                                    refinementPos[cntr].insert(d);
                                }
                            }
                        }
                        for (const auto& r : refinementPos)
                        {
                            std::map<uint16_t, Position> t = planData.at(r.first);
                            for (const auto& droneID : r.second)
                            {
                                t[droneID] = targets[droneID];
                            }
                            std::map<uint16_t, NodeType> n;
                            for (const auto& x : nodePlan[r.first])
                            {
                                n[x.first] = x.second;
                            }
                            t = this->geometry.refine<uint16_t>(t, n);

                            for (const auto& x : t)
                            {
                                planData[r.first][x.first] = x.second;
                            }
                        }
                    }
#endif

                    this->plan.emplace(Plan(*this, planData));

                    MSG_INFO("Plan serialized");
                }
            }
            else
            {
                /*There is a drone which has a setpoint but is not contained in the drones reported by the drone swarm interface -> Ignore request for now*/
                MSG_WARNING("Invalid drone setpoints (there is a drone with a setpoint that does not exist!)");
            }
        }
        break;
        case SWARM_IDLE:
        case SWARM_LANDING:
        case SWARM_TAKING_OFF:
        case SWARM_STOPPING:
        {
            /*Ignore*/
            MSG_WARNING("Request ignored due to state!");
        }
        break;
    }
}
SwarmOperationHandler::Plan::Plan(SwarmOperationHandler& pSwarmOperationHandler, const std::vector<std::map<uint16_t, Position>>& pSteps)
: swarmOperationHandler(pSwarmOperationHandler), currentState(Plan::State::PLAN_INITIALIZE), steps(pSteps), currentStep(steps.begin()), lastStep(steps.begin())
{

}
SwarmOperationHandler::Plan::Plan(const Plan& pOther)
: swarmOperationHandler(pOther.swarmOperationHandler), currentState(pOther.currentState), steps(pOther.steps), currentStep(steps.begin()), lastStep(steps.begin())
{

}
void SwarmOperationHandler::Plan::update()
{
    static Plan::State lastState = PLAN_DONE;
    static uint32_t printTimer = 0;
    static std::string stateNames[] = {"Initialize", "Wait for first hover", "Send next targets", "Wait for target", "Wait for hover", "Done"};
    static uint32_t targetSentTime = 0;

    if (getTimedif(printTimer, getMillis()) >= 500 || this->currentState != lastState)
    {
        MSG_INFO("Current plan state: " + stateNames[this->currentState]);
        lastState = currentState;
        printTimer = getMillis();
    }
    switch(this->currentState)
    {
        case Plan::PLAN_INITIALIZE:
        {
            std::string planStr = "Steps:\n";
            unsigned int cntr = 0;
            for(const auto& s : steps)
            {
                planStr += "\t" + cntr;
                for(const auto& a : s)
                {
                    planStr += std::to_string(a.first) + ": " + (std::string)a.second + "\t";
                }
                cntr++;
                planStr += "\n";
            }
            planStr += "\n";
            MSG_INFO(planStr);
            this->currentStep = this->steps.begin();
            this->lastStep = this->currentStep;
            targetSentTime = getMillis();
            this->currentState = Plan::PLAN_WAIT_FOR_FIRST_HOVER;
        }
        break;
        case Plan::PLAN_WAIT_FOR_FIRST_HOVER:
        {
            if (this->swarmOperationHandler.droneSwarmInterfaceClient.getSwarmState() == SWARM_HOVERING && getTimedif(targetSentTime, getMillis()) >= 200)
            {
                MSG_INFO("Hovering detected -> Issue first step (first target position)");
                this->currentState = Plan::PLAN_SEND_NEXT_TARGETS;
            }
        }
        break;
        case Plan::PLAN_SEND_NEXT_TARGETS:
        {
            this->getCurrentStep();
            if(this->currentStep == this->steps.end())
            {
                MSG_INFO("Plan done");
                this->currentState = Plan::PLAN_DONE;
            }
            else
            {
                this->swarmOperationHandler.droneSwarmInterfaceClient.updateDroneTargets(*this->currentStep);

                std::map<uint16_t, DroneOperation> operations;
                std::set<uint16_t> drones = this->swarmOperationHandler.droneSwarmInterfaceClient.getDrones();
                for(const auto& drone : drones)
                {
                    /*Make all drones move simultaniously*/
                    if (this->currentStep != this->lastStep)
                    {
                        if (this->currentStep->at(drone) != this->lastStep->at(drone))
                        {
                            operations.insert(std::make_pair(drone, DRONE_OPERATION_MOVE));
                        }
                        else
                        {
                            operations.insert(std::make_pair(drone, DRONE_OPERATION_NONE));
                        }
                    }
                    else
                    {
                        operations.insert(std::make_pair(drone, DRONE_OPERATION_MOVE));
                    }
                }
                this->swarmOperationHandler.droneSwarmInterfaceClient.updateDroneOperations(operations);

                MSG_INFO("Updated drone targets according to plan");
                
                this->currentState = Plan::PLAN_WAIT_FOR_TARGET;
                targetSentTime = getMillis();
            }
        }
        break;
        case Plan::PLAN_WAIT_FOR_TARGET:
        {
            std::map<uint16_t, Position> dronePos = this->swarmOperationHandler.droneSwarmInterfaceClient.getDronePositions();

            if (std::none_of(dronePos.begin(), dronePos.end(), [&](const std::pair<uint16_t, Position>& kvp) {
                return kvp.second.getEuclideanDistance(currentStep->at(kvp.first)) > 0.1;
            }))
            {
                MSG_INFO("Targets reached -> Wait for hover (after 200ms)");
                this->currentState = Plan::PLAN_WAIT_FOR_HOVER;
            }
        }
        break;
        case Plan::PLAN_WAIT_FOR_HOVER:
        {
            if(this->swarmOperationHandler.droneSwarmInterfaceClient.getSwarmState() == SWARM_HOVERING && getTimedif(targetSentTime, getMillis()) >= 200)
            {
                this->lastStep = currentStep;
                this->currentStep++;

                if (this->currentStep == this->steps.end())
                {
                    MSG_INFO("Plan done");
                    this->currentState = Plan::PLAN_DONE;
                }
                else
                {
                    std::string nextStep = "Hovering detected -> Issue next step: ";

                    for (const auto& a : *(this->currentStep))
                    {
                        nextStep += std::to_string(a.first) + ": " + (std::string)a.second + "\t";
                    }

                    MSG_INFO(nextStep);

                    this->currentState = Plan::PLAN_SEND_NEXT_TARGETS;
                }
            }
        }
        break;
        case Plan::PLAN_DONE:
        {
            
        }
        break;
    }
}
const std::vector<std::map<uint16_t, Position>>& SwarmOperationHandler::Plan::getSteps() const
{
    return this->steps;
}
SwarmOperationHandler::Plan::State SwarmOperationHandler::Plan::getState() const
{
    return this->currentState;
}
std::map<uint16_t, Position> SwarmOperationHandler::Plan::getCurrentStep() const
{
    if (this->currentStep != this->steps.end())
    {
        return *(this->currentStep);
    }
    else
    {
        return std::map<uint16_t, Position>();
    }
}


void SwarmOperationHandler::update()
{
    std::optional<SwarmOperation> operation = this->interactionServer.peekRequest();
    if(operation.has_value())
    {
        MSG_INFO("Request received: " + std::to_string(operation.value()));
        this->interactionServer.dequeueRequest();
        switch(operation.value())
        {
            case SWARM_OPERATION_FAST_STOP:
            {
                this->handleFastStopRequest();
            }
            break;
            case SWARM_OPERATION_LAND:
            {
                this->handleLandRequest();
            }
            break;
            case SWARM_OPERATION_TAKEOFF:
            {
                this->handleTakeoffRequest();
            }
            break;
            case SWARM_OPERATION_MOVE:
            {
                this->handleMoveRequest();
            }
            break;
        }
    }

    std::map<uint16_t, Position> pos = this->droneSwarmInterfaceClient.getDronePositions();
    for (const auto& p : pos)
    {
        for (const auto& p2 : pos)
        {
            if (p.first != p2.first && p.second.getEuclideanDistance(p2.second) <= 0.2)
            {
                std::string msg = "The drones " + std::to_string(p.first) + " and " + std::to_string(p2.first) + "were close to colliding! (distance: " + std::to_string(p.second.getEuclideanDistance(p2.second)) + ")!";
                MSG_ERROR(msg);
                this->handleFastStopRequest();
            }
        }
    }

    if(this->plan.has_value())
    {
        this->plan.value().update();
        if(this->plan.value().getState() == SwarmOperationHandler::Plan::State::PLAN_DONE)
        {
            this->plan.reset();
        }
    }

    this->interactionServer.updateSwarmState(this->droneSwarmInterfaceClient.getSwarmState());
    this->interactionServer.updateDronePositions(this->droneSwarmInterfaceClient.getDronePositions());
    this->interactionServer.updateDroneStates(this->droneSwarmInterfaceClient.getDroneStates());
}
SwarmOperationHandler::SwarmOperationHandler(InteractionServer& pInteractionServer, DroneSwarmInterfaceClient& pDroneSwarmInterfaceClient, GeometryModule& pGeometry)
: interactionServer(pInteractionServer), droneSwarmInterfaceClient(pDroneSwarmInterfaceClient), geometry(pGeometry), plan()
{

}