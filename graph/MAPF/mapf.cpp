/**
 * @file mapf.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains implementations of functions used in MAPF problems
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "mapf.hpp"

using namespace MAPF;

Task::Task(Graph& pGraph, const std::map<unsigned int, std::pair<NodeType, NodeType>>& pAgentsStartTarget)
: agentsStartTarget(pAgentsStartTarget), graph(pGraph)
{

}
Task::Task(Graph& pGraph)
: agentsStartTarget(), graph(pGraph)
{

}
void Task::addAgent(unsigned int pAgent, NodeType pStart, NodeType pEnd)
{
    this->agentsStartTarget[pAgent] = std::make_pair(pStart, pEnd);
}
void Task::eraseAgent(unsigned int pAgent)
{
    this->agentsStartTarget.erase(pAgent);
}
std::set<unsigned int> Task::getAgents() const
{
    std::set<unsigned int> result;
    for(const auto& p : this->agentsStartTarget)
    {
        result.insert(p.first);
    }
    return result;
}
NodeType Task::getAgentStart(unsigned int pAgent) const
{
    return this->agentsStartTarget.at(pAgent).first;
}
NodeType Task::getAgentTarget(unsigned int pAgent) const
{
    return this->agentsStartTarget.at(pAgent).second;
}
std::map<unsigned int, std::pair<NodeType, NodeType>> Task::getAgentsStartTarget() const
{
    return this->agentsStartTarget;
}
const Graph& Task::getGraph() const
{
    return this->graph;
}

Plan::Plan(std::map<unsigned int, std::map<unsigned int, NodeType>> pSteps)
: steps(pSteps)
{

}
void Plan::simulate(std::function<void(const std::map<unsigned int, NodeType>&)> pCallback)
{
    std::map<unsigned int, NodeType> currentPositions;
    unsigned int t = 0;
    for(t=0; t<=this->steps.rbegin()->first; t++)
    {
        for(const auto& a : this->steps.at(t))
        {
            currentPositions[a.first] = a.second;
        }
        pCallback(currentPositions);
    }
}