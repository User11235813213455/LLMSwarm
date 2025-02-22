/**
 * @file ConstraintTree.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains implementations of functions used to manage constraint trees as used in CBS (collision based search)
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "ConstraintTree.hpp"
#include <optional>
#include <iostream>

#define TEST_PATHFINDING 0
#if TEST_PATHFINDING
#warning "You enabled testing of the low level path finding algorithm, make sure to deactivate it once you don't need it anymore as it affects performance heavily"
#endif

static size_t hashCombine(size_t lhs, size_t rhs) 
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
};

Constraint::Constraint(std::tuple<unsigned int, unsigned int, NodeType> pTuple) : t(pTuple)
{
}
Constraint::Constraint(unsigned int pTimestep, unsigned int pAgent, NodeType pNode) : t(std::make_tuple(pTimestep, pAgent, pNode))
{

}
unsigned int Constraint::getAgent() const
{
    return std::get<1>(this->t);
}
unsigned int Constraint::getTimestep() const
{
    return std::get<0>(this->t);
}
NodeType Constraint::getNode() const
{
    return std::get<2>(this->t);
}
size_t Constraint::hash() const
{
    return hashCombine(std::hash<NodeType>{}(this->getNode()), 
    hashCombine(std::hash<unsigned int>{}(this->getAgent()), 
    std::hash<unsigned int>{}(this->getTimestep())));
}
bool Constraint::operator<(const Constraint& pOther) const
{
    if(this->getTimestep() > pOther.getTimestep())
    {
        return true;
    }
    else if(this->getTimestep() < pOther.getTimestep())
    {
        return false;
    }
    else
    {
        if(this->getNode() < pOther.getNode())
        {
            return true;
        }
        else if(this->getNode() > pOther.getNode())
        {
            return false;
        }
        else
        {
            return this->getAgent() < pOther.getAgent();
        }
    }
}
Conflict::Conflict(unsigned int pTimestep, unsigned int pAgent1, unsigned int pAgent2, NodeType pNode1, NodeType pNode2) 
: t(std::make_tuple(pTimestep, pAgent1, pAgent2, pNode1, pNode2))
{

}
unsigned int Conflict::getTimestep() const
{
    return std::get<0>(this->t);
}
unsigned int Conflict::getAgent1() const
{
    return std::get<1>(this->t);
}
unsigned int Conflict::getAgent2() const
{
    return std::get<2>(this->t);
}
NodeType Conflict::getNode1() const
{
    return std::get<3>(this->t);
}
NodeType Conflict::getNode2() const
{
    return std::get<4>(this->t);
}

ConstraintTree::ConstraintTree(const Graph& pGraph, const std::map<unsigned int, std::pair<NodeType, NodeType>>& pAgentTasks, std::function<double(NodeType, NodeType)> pH) 
: agentTasks(pAgentTasks), graph(pGraph), costSum(0.0), hashValue(0)
{
    /*Root node -> calculate a whole new solution*/
    this->calculateSolution(pH);
}

ConstraintTree::ConstraintTree(const ConstraintTree& pParent, Constraint pConstraint, std::function<double(NodeType, NodeType)> pH) 
: constraints(pParent.constraints), solution(pParent.solution), agentTasks(pParent.agentTasks), graph(pParent.graph), costs(pParent.costs),
  costSum(pParent.costSum), hashValue(0)
{
    /*Add one constraint to the list as a conflict occured on the parent*/
    this->constraints[pConstraint.getAgent()][pConstraint.getTimestep()].insert(pConstraint.getNode());

    /*Recalculate the path and the cost for one agent*/
    this->updateSolution(pConstraint.getAgent(), pH);
}
std::optional<Conflict> ConstraintTree::getFirstConflict() const
{
    if(this->solution.size() <= 1)
    {
        /*There can't be any conflict per definition*/
        return {};
    }

    unsigned int timeCntr = 1;
    /*Count until the maximum time; A std::map is sorted thus take the last element as maximum;
    We can start at 1, as the initial position does not matter and can not cause a conflict;
    Search for node conflicts first*/
    for(timeCntr=1; timeCntr <= this->solution.rbegin()->first; timeCntr++)
    {
        /*Invert the mapping to detect collisions*/
        std::map<NodeType, unsigned int> nodeAssignments;
        if(this->solution.count(timeCntr) > 0)
        {
            if(this->solution.at(timeCntr).size() < this->agentTasks.size())
            {
                /*At least one agent has to be missing*/
                throw(std::runtime_error("Invalid solution: Not all agents have an assignment at at least one timestep!"));
            }
            for(const auto& agentPos : this->solution.at(timeCntr))
            {
                if(nodeAssignments.count(agentPos.second) > 0)
                {
                    /*Collision as the node is already taken -> return the conflict*/
                    return Conflict(timeCntr, nodeAssignments.at(agentPos.second), agentPos.first, agentPos.second, agentPos.second);
                }
                else
                {
                    /*Store the assignment of the current agent to the node*/
                    nodeAssignments[agentPos.second] = agentPos.first;
                }
            }
        }
        else
        {
            throw(std::runtime_error("Invalid solution: interrupted timeline!"));
        }
    }
    /*Check swap/edge conflicts; We need to start at 0 now to check for swapping at the beginning
    but we only have to consider all steps before the last one as at least one step is necessary
    to create such collisions*/
    for(timeCntr=0; timeCntr + 1 <= this->solution.rbegin()->first; timeCntr++)
    {
        /*No need to check for existence as we already did that in the previous loop*/
        const std::map<unsigned int, NodeType>& step = this->solution.at(timeCntr);
        const std::map<unsigned int, NodeType>& next = this->solution.at(timeCntr+1);

        for(const auto& agent : this->agentTasks)
        {
            for(const auto& agent2 : this->agentTasks)
            {
                if(agent == agent2)
                {
                    continue;
                }
                else
                {
                    if(step.at(agent.first) == next.at(agent2.first) && next.at(agent.first) == step.at(agent2.first))
                    {
                        /*They switched positions -> report the conflict; Read as: Either agent.first shall not be on the node
                        next.at(agent.first) or agent2 shall not be on the node step.at(agent.first) at timestep timeCntr*/
                        return Conflict(timeCntr + 1, agent.first, agent2.first, next.at(agent.first), step.at(agent.first));
                    }
                }
            }
        }
    }

    /*Collision free*/
    return {};
}
void ConstraintTree::calculateSolution(std::function<double(NodeType, NodeType)> pH)
{
    /*Reinitialize the sum with 0 as we recalculate it*/
    this->costSum = 0.0;
    this->solution.clear();

    for(const std::pair<unsigned int, std::pair<NodeType, NodeType>>& currentAgentTask : this->agentTasks)
    {
        /*For every agent calculate a new path which mets the constraints of this tree node*/
        std::vector<NodeType> path = this->graph.getShortestPath(currentAgentTask.second.first, 
                                                                currentAgentTask.second.second, 
                                                                pH, 
                                                                std::set<NodeType>(), 
                                                                getConstraintsForAgent(currentAgentTask.first));
        if(path.empty())
        {
            this->solution.clear();
        }
        #if TEST_PATHFINDING
        if(!this->validateLowLevelPathfinding(currentAgentTask.first, path))
        {
            throw("The low level path finding algorithm calculated a path which ignores at least one constraint!");
        }
        #endif

        this->costs[currentAgentTask.first] = this->graph.getPathCost(path);
        this->costSum += this->costs[currentAgentTask.first];

        unsigned int t;
        for(t = 0; t < path.size(); t++)
        {
            this->solution[t][currentAgentTask.first] = path[t];
        }
    }
    /*Fill up the paths of all agents until they are of the same length (if an entry is missing it means that the agent
    waited on the node he was before as his path was shorter)*/
    this->pumpUpSolution();
    
    this->hashValue = this->hash();
}
void ConstraintTree::updateSolution(unsigned int pAgent, std::function<double(NodeType, NodeType)> pH)
{
    for(std::pair<const unsigned int, std::map<unsigned int, NodeType>>& step : this->solution)
    {
        step.second.erase(pAgent);
    }

    std::pair<NodeType, NodeType> task = this->agentTasks.at(pAgent);
    std::vector<NodeType> path = this->graph.getShortestPath(task.first, 
                                                            task.second, 
                                                            pH, 
                                                            std::set<NodeType>(),
                                                            getConstraintsForAgent(pAgent));
    if(path.empty())
    {
        this->solution.clear();
    }
    else
    {
        #if TEST_PATHFINDING
        if(!this->validateLowLevelPathfinding(pAgent, path))
        {
            path = this->graph.getShortestPath(task.first, 
                                                            task.second, 
                                                            pH, 
                                                            std::set<NodeType>(),
                                                            getConstraintsForAgent(pAgent));
            throw("The low level path finding algorithm calculated a path which ignores at least one constraint!");
        }
        #endif

        double oldCost = this->costs[pAgent];
        this->costs[pAgent] = this->graph.getPathCost(path);
        this->costSum += (this->costs[pAgent] - oldCost);

        unsigned int t;
        for(t = 0; t < path.size(); t++)
        {
            this->solution[t][pAgent] = path[t];
        }

        this->pumpUpSolution();

        this->hashValue = this->hash();
    }
}
void ConstraintTree::printConstraints() const
{
    std::cout << "{";
    for(const auto& c : this->constraints)
    {
        std::cout << "(" << c.first << ": ";
        for(const auto& c2 : c.second)
        {
            std::cout << "(" << c2.first << ": {";
            for(const auto& c3 : c2.second)
            {
                std::cout << c3 << " ";
            }
            std::cout << "})";
        }
        std::cout << ") ";
    }
    std::cout << "}" << std::endl;
}
std::map<unsigned int, std::set<NodeType>> ConstraintTree::getConstraintsForAgent(unsigned int pAgent) const
{
    if(this->constraints.count(pAgent) > 0)
    {
        return this->constraints.at(pAgent);
    }
    else
    {
        return std::map<unsigned int, std::set<NodeType>>();
    }
}
size_t ConstraintTree::hash() const
{
    size_t last = 0;
    bool first = true;
    for(const auto& agent : this->constraints)
    {
        for(const auto& timestep : this->constraints.at(agent.first))
        {
            for(const auto& node : this->constraints.at(agent.first).at(timestep.first))
            {
                Constraint c(timestep.first, agent.first, node);
                if(first)
                {
                    last = c.hash();
                    first = false;
                }
                else
                {
                    last = hashCombine(last, c.hash());
                }
            }
        }
    }
    return last;
}
bool ConstraintTree::validateLowLevelPathfinding(unsigned int pAgent, const std::vector<NodeType>& pPath) const
{
    unsigned int cntr = 0;
    std::map<unsigned int, std::set<NodeType>> constraints = this->getConstraintsForAgent(pAgent);
    if(constraints.empty())
    {
        return true;
    }
    for(cntr=0; cntr<pPath.size(); cntr++)
    {
        NodeType node = pPath[cntr];
        if(constraints.count(cntr) > 0)
        {
            if(constraints[cntr].count(node) > 0)
            {
                return false;
            }
        }
    }
    return true;
}
size_t ConstraintTree::getHash() const
{
    return this->hashValue;
}
void ConstraintTree::pumpUpSolution()
{
    unsigned int timeCntr = 1;
    for(timeCntr=1; timeCntr <= this->solution.rbegin()->first; timeCntr++)
    {
        if(this->solution.count(timeCntr) > 0)
        {
            for(const auto& agent : this->agentTasks)
            {
                if(this->solution.at(timeCntr).count(agent.first) == 0)
                {
                    this->solution.at(timeCntr)[agent.first] = this->solution.at(timeCntr-1).at(agent.first);
                }
            }
        }
        else
        {
            throw(std::runtime_error("Invalid solution: Timeline interrupted!"));
        }
    }
}
void ConstraintTree::addConstraint(Constraint pConstraint)
{
    this->constraints[pConstraint.getAgent()][pConstraint.getTimestep()].insert(pConstraint.getNode());
}
std::map<unsigned int, std::map<unsigned int, NodeType>> ConstraintTree::getSolution() const
{
    return this->solution;
}
double ConstraintTree::getCostSum() const
{
    return this->costSum;
}
bool ConstraintTree::operator<(const ConstraintTree& pOther) const
{
    if(this->costSum < pOther.costSum)
    {
        return true;
    }
    else if(this->costSum == pOther.costSum)
    {
        if(this->hashValue < pOther.hashValue)
        {
            return true;
        }
        else if(this->hashValue == pOther.hashValue)
        {
            /*Maybe equal*/
            std::set<Constraint> thisConstraints;
            std::set<Constraint> otherConstraints;

            for(const auto& agent : this->constraints)
            {
                for(const auto& timestep : this->constraints.at(agent.first))
                {
                    for(const auto& node : this->constraints.at(agent.first).at(timestep.first))
                    {
                        thisConstraints.insert(Constraint(timestep.first, agent.first, node));
                    }
                }
            }
            for(const auto& agent : pOther.constraints)
            {
                for(const auto& timestep : pOther.constraints.at(agent.first))
                {
                    for(const auto& node : pOther.constraints.at(agent.first).at(timestep.first))
                    {
                        otherConstraints.insert(Constraint(timestep.first, agent.first, node));
                    }
                }
            }

            std::set<Constraint>::iterator c1 = thisConstraints.begin();
            std::set<Constraint>::iterator c2 = otherConstraints.begin();

            while(true)
            {
                if(c1 == thisConstraints.end())
                {
                    if(c2 == otherConstraints.end())
                    {
                        /*Equal -> return false*/
                        return false;
                    }
                    else
                    {
                        return true;
                    }
                }
                else if(c2 == otherConstraints.end())
                {
                    /*c1 is not at end -> c2 has less elements and is thus "smaller"*/
                    return false;
                }
                else
                {

                    if(*c1 < *c2)
                    {
                        return true;
                    }
                    else if(*c2 < *c1)
                    {
                        return false;
                    }
                    else
                    {
                        c1++;
                        c2++;
                    }
                }
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
bool ConstraintTree::hasSolution() const
{
    return !this->solution.empty();
}