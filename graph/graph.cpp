/**
 * @file graph.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains implementations of different graph algorithms such as obstacle avoidance, path finding (A*, Dijkstra)
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "graph.hpp"
#include <queue>
#include <algorithm>
#include <numeric>
#include <cstdlib>

#define TEST_PATHFINDING 0
#if TEST_PATHFINDING
#warning "You enabled testing of the low level path finding algorithm, make sure to deactivate it once you don't need it anymore as it affects performance heavily"
#endif

Graph::Graph(std::initializer_list<NodeType> pNodes, std::initializer_list<std::tuple<NodeType, NodeType, double>> pEdges)
{
    for(auto n : pNodes)
    {
        this->nodes.insert(n);
        this->edges[n] = std::set<NodeType>();
        this->edgesIn[n] = std::set<NodeType>();
    }
    for(auto e : pEdges)
    {
        if(this->nodes.count(std::get<0>(e)) > 0 && this->nodes.count(std::get<1>(e)) > 0)
        {
            this->edges.at(std::get<0>(e)).insert(std::get<1>(e));
            this->edgesIn.at(std::get<1>(e)).insert(std::get<0>(e));
            this->weights[std::make_pair(std::get<0>(e), std::get<1>(e))] = std::get<2>(e);
        }
    }
}
Graph::Graph(std::set<NodeType> pNodes, std::set<std::tuple<NodeType, NodeType, double>> pEdges)
{
    for(auto n : pNodes)
    {
        this->nodes.insert(n);
        this->edges[n] = std::set<NodeType>();
        this->edgesIn[n] = std::set<NodeType>();
    }
    for(auto e : pEdges)
    {
        if(this->nodes.count(std::get<0>(e)) > 0 && this->nodes.count(std::get<1>(e)) > 0)
        {
            this->edges.at(std::get<0>(e)).insert(std::get<1>(e));
            this->edgesIn.at(std::get<1>(e)).insert(std::get<0>(e));
            this->weights[std::make_pair(std::get<0>(e), std::get<1>(e))] = std::get<2>(e);
        }
    }
}
Graph::Graph()
{
    /*Do nothing*/
}
bool Graph::addNode(NodeType pNode)
{
    if(this->nodes.count(pNode) > 0)
    {
        return false;
    }
    else
    {
        this->nodes.insert(pNode);
        this->edges[pNode] = std::set<NodeType>();
        this->edgesIn[pNode] = std::set<NodeType>();
        return true;
    }
}
bool Graph::addEdge(std::pair<NodeType, NodeType> pEdge, double pWeight)
{
    if(this->nodes.count(pEdge.first) > 0 && this->nodes.count(pEdge.second) > 0)
    {
        std::set<NodeType>& e = this->edges.at(pEdge.first);
        if(e.count(pEdge.second) == 0)
        {
            e.insert(pEdge.second);
            this->edgesIn.at(pEdge.second).insert(pEdge.first);
            this->weights[std::make_pair(pEdge.first, pEdge.second)] = pWeight;
            return true;
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
bool Graph::addEdge(NodeType pFrom, NodeType pTo, double pWeight)
{
    return this->addEdge(std::make_pair(pFrom, pTo), pWeight);
}
bool Graph::addEdge(std::tuple<NodeType, NodeType, double> pEdge)
{
    return this->addEdge(std::make_pair(std::get<0>(pEdge), std::get<1>(pEdge)), std::get<2>(pEdge));
}
bool Graph::removeNode(NodeType pNode)
{
    if(this->nodes.count(pNode) > 0)
    {
        this->nodes.erase(pNode);
        std::set<NodeType>& outgoingEdges = this->edges.at(pNode);
        for(NodeType n : outgoingEdges)
        {
            this->weights.erase(std::make_pair(pNode, n));
        }
        outgoingEdges.clear();
        this->edges.erase(pNode);
        for(std::pair<NodeType, std::set<NodeType>> i : this->edgesIn)
        {
            this->edgesIn.at(i.first).erase(pNode);
        }
        return true;
    }
    else
    {
        return false;
    }
}
bool Graph::removeEdge(std::pair<NodeType, NodeType> pEdge)
{
    if(this->nodes.count(pEdge.first) > 0 && this->nodes.count(pEdge.second) > 0)
    {
        std::set<NodeType>& e = this->edges.at(pEdge.first);
        if(e.count(pEdge.second) > 0)
        {
            e.erase(pEdge.second);
            this->edgesIn.at(pEdge.second).erase(pEdge.first);
            this->weights.erase(std::make_pair(pEdge.first, pEdge.second));
            return true;
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
std::set<NodeType> Graph::getOutgoingEdges(NodeType pNode) const
{
    if(this->nodes.count(pNode) > 0)
    {
        return this->edges.at(pNode);
    }
    else
    {
        return std::set<NodeType>();
    }
}
std::set<NodeType> Graph::getIncomingEdges(NodeType pNode) const
{
    if(this->nodes.count(pNode) > 0)
    {
        return this->edgesIn.at(pNode);
    }
    else
    {
        return std::set<NodeType>();
    }
}
std::set<NodeType> Graph::getNodes() const
{
    return this->nodes;
}
double Graph::getWeight(std::pair<NodeType, NodeType> pEdge) const
{
    if(this->weights.count(pEdge) > 0)
    {
        return this->weights.at(pEdge);
    }
    else
    {
        return 0.0;
    }
}
double Graph::getWeight(NodeType pStart, NodeType pEnd) const
{
    return this->getWeight(std::make_pair(pStart, pEnd));
}
std::map<NodeType, double> Graph::getOutgoingEdgesWithWeights(NodeType pNode) const
{
    if(this->nodes.count(pNode) > 0)
    {
        std::map<NodeType, double> result;
        const std::set<NodeType>& outgoing = this->edges.at(pNode);
        for(NodeType n : outgoing)
        {
            result[n] = this->weights.at(std::make_pair(pNode, n));
        }
        return result;
    }
    else
    {
        return std::map<NodeType, double>();
    }
}
std::map<NodeType, double> Graph::getIncomingEdgesWithWeights(NodeType pNode) const
{
    if(this->nodes.count(pNode) > 0)
    {
        std::map<NodeType, double> result;
        const std::set<NodeType>& incoming = this->edgesIn.at(pNode);
        for(NodeType n : incoming)
        {
            result[n] = this->weights.at(std::make_pair(pNode, n));
        }
        return result;
    }
    else
    {
        return std::map<NodeType, double>();
    }
}
std::map<NodeType, std::vector<NodeType>> Graph::getAllShortestPaths(NodeType pStart, std::set<NodeType> pObstacles) const
{
    if(this->nodes.count(pStart) == 0 || pObstacles.count(pStart) > 0)
    {
        return std::map<NodeType, std::vector<NodeType>>();
    }

    std::map<NodeType, NodeType> predecessor;
    std::map<NodeType, double> Q;

    for(NodeType n : this->nodes)
    {
        if(pObstacles.count(n) > 0)
        {
            continue;
        }
        if(n == pStart)
        {
            Q[n] = 0.0;
        }
        else
        {
            Q[n] = std::numeric_limits<double>::infinity();
        }
        
    }

    while(!Q.empty())
    {
        NodeType u = std::min_element(Q.begin(), Q.end(), [](std::pair<const NodeType, double> p1, std::pair<const NodeType, double> p2) { return p1.second < p2.second; } )->first;
        double distance = Q[u];
        Q.erase(u);
        const std::set<NodeType>& neighbours = this->edges.at(u);
        for(NodeType v : neighbours)
        {
            if(pObstacles.count(v))
            {
                continue;
            }
            if(Q.count(v) > 0)
            {
                double alt = distance + this->weights.at(std::make_pair(u, v));
                if(alt < Q[v])
                {
                    Q[v] = alt;
                    predecessor[v] = u;
                }
            }
        }
    }

    std::map<NodeType, std::vector<NodeType>> result;
    for(NodeType n : this->nodes)
    {
        if(pObstacles.count(n))
        {
            continue;
        }
        if(n == pStart)
        {
            continue;
        }
        std::vector<NodeType> path;
        NodeType currentNode = n;
        while(currentNode != pStart)
        {
            path.push_back(currentNode);
            currentNode = predecessor[currentNode];
        }
        path.push_back(pStart);
        std::reverse(path.begin(), path.end());
        result[n] = path;
    }
    return result;
}
Graph::Graph(const Graph& pOther)
{
    this->edges = pOther.edges;
    this->edgesIn = pOther.edges;
    this->nodes = pOther.nodes;
    this->weights = pOther.weights;
}
Graph& Graph::operator=(Graph& pOther)
{
    this->edges = pOther.edges;
    this->edgesIn = pOther.edges;
    this->nodes = pOther.nodes;
    this->weights = pOther.weights;
    return *this;
}
Graph::~Graph()
{
    this->edges.clear();
    this->edgesIn.clear();
    this->nodes.clear();
    this->weights.clear();
}
std::map<NodeType, std::pair<std::vector<NodeType>, double>> Graph::getAllShortestPathsWithCosts(NodeType pStart, std::set<NodeType> pObstacles) const
{
    std::map<NodeType, std::vector<NodeType>> shortestPaths = this->getAllShortestPaths(pStart, pObstacles);
    std::map<NodeType, std::pair<std::vector<NodeType>, double>> result;

    while(!shortestPaths.empty())
    {
        std::pair<NodeType, std::vector<NodeType>> p = *shortestPaths.begin();
        double sum = 0.0;
        NodeType lastNode = pStart;
        std::for_each(p.second.begin() + 1, p.second.end(), [&](const NodeType& pNode) { sum += this->weights.at(std::make_pair(lastNode, pNode)); lastNode = pNode; });
        result[p.first] = std::make_pair(p.second, sum);
        shortestPaths.erase(p.first);
    }
    return result;
}
bool Graph::checkPathConstraints(const std::vector<NodeType>& pPath, const std::map<unsigned int, std::set<NodeType>> pConstraints) const
{
    unsigned int cntr = 0;
    for(cntr=0; cntr<pPath.size(); cntr++)
    {
        if(pConstraints.count(cntr) > 0)
        {
            if(pConstraints.at(cntr).count(pPath[cntr]) > 0)
            {
                return false;
            }
        }
    }
    return true;
}
struct State
{
    unsigned int timestep;
    NodeType node;
    double f;

    State(NodeType pNode, unsigned int pTimestep, double pF)
    {
        this->node = pNode;
        this->timestep = pTimestep;
        this->f = pF;
    }
    bool operator<(const State& pOther) const 
    {
        if(this->f < pOther.f)
        {
            return true;
        }
        else if(this->f == pOther.f)
        {
            if(this->timestep < pOther.timestep)
            {
                return true;
            }
            else if(this->timestep == pOther.timestep)
            {
                return this->node < pOther.node;
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
    State& operator=(const State& pOther)
    {
        this->f = pOther.f;
        this->node = pOther.node;
        this->timestep = pOther.timestep;
        return *this;
    }
};
bool operator<(const std::pair<unsigned int, NodeType>& p1, const std::pair<unsigned int, NodeType>& p2)
{
    return p1.first <= p2.first || (p1.first == p2.first && p1.second < p2.second);
}
std::vector<NodeType> Graph::getShortestPath(NodeType pStart, NodeType pTarget, std::function<double(NodeType, NodeType)> pH, std::set<NodeType> pObstacles, std::map<unsigned int, std::set<NodeType>> pConstraints) const
{
    if(this->nodes.count(pStart) == 0 || this->nodes.count(pTarget) == 0 || pObstacles.count(pStart) > 0)
    {
        return std::vector<NodeType>();
    }

    std::set<State> openList;
    std::map<unsigned int, std::set<NodeType>> closedList;
    std::map<std::pair<unsigned int, NodeType>, std::pair<unsigned int, NodeType>> predecessor;
    std::map<NodeType, double> g;

    for(const NodeType& n : this->nodes)
    {
        if(pObstacles.count(n) > 0)
        {
            /*Skip obstacles*/
            continue;
        }
        g[n] = 0.0;
    }

    openList.insert(State(pStart, 0, 0));
    
    do
    {
        State currentState = *openList.begin();
        openList.erase(currentState);
        
        if(currentState.node == pTarget)
        {
            /*We are at our target position; Check if there are future constraints which forbid staying here*/
            bool constrained = false;

            if(pConstraints.size() > 0)
            {
                unsigned int t = 0;
                for(t = currentState.timestep; t <= pConstraints.rbegin()->first; t++)
                {
                    if(pConstraints[t].contains(pTarget))
                    {
                        /*We might be on the right node but we are not allowed to stay -> continue A**/
                        constrained = true;
                        break;
                    }
                }
            }
            if(!constrained)
            {
                std::vector<NodeType> result;
                std::pair<unsigned int, NodeType> c = std::make_pair(currentState.timestep, currentState.node);
                while(predecessor.contains(c))
                {
                    //std::cout << c.first << ": " << c.second << std::endl;
                    result.push_back(c.second);
                    c = predecessor[c];
                }
                result.push_back(pStart);
                std::reverse(result.begin(), result.end());
                return result;
            }
        }

        closedList[currentState.timestep].insert(currentState.node);

        /*Expand*/
        std::set<NodeType> successors;
        if(this->edges.contains(currentState.node))
        {
            successors = this->edges.at(currentState.node);
        }
        for(NodeType s : successors)
        {
            if(pObstacles.count(s) > 0)
            {
                /*Skip obstacles*/
                continue;
            }
            if(pConstraints.contains(currentState.timestep+1) && pConstraints.at(currentState.timestep+1).contains(s))
            {
                continue;
            }

            if(closedList.contains(currentState.timestep+1) && closedList.at(currentState.timestep+1).contains(s))
            {
                /*In closed list*/
                continue;
            }

            double tentativeG = g[currentState.node] + this->weights.at(std::make_pair(currentState.node, s));

            std::set<State>::iterator l = std::find_if(openList.begin(), openList.end(), [&](const State& a) { return a.node == s; });

            if(l != openList.end() && tentativeG >= g[s])
            {
                continue;
            }

            predecessor[std::make_pair(currentState.timestep + 1, s)] = std::make_pair(currentState.timestep, currentState.node);
            g[s] = tentativeG;
            /*calculate the heuristic for the original (non virtual) successor s*/
            double fValue = tentativeG + pH(s, pTarget);
            if(l != openList.end())
            {
                openList.erase(l);
                openList.insert(State(s, currentState.timestep+1, fValue));
            }
            else
            {
                openList.insert(State(s, currentState.timestep+1, fValue));
            }
        }
    } while(!openList.empty());

    return std::vector<NodeType>();
}
NodeType Graph::generateNewNode() const
{
    std::string str;
    do
    {
        str = "";
        unsigned int cntr;
        for(cntr=0; cntr<20; cntr++)
        {
            str.append({(char)(rand() % 10 + 48)});
        }
    } while(this->nodes.count(str) > 0);
    
    return str;
}
double Graph::getPathCost(const std::vector<NodeType>& pPath) const
{
    if(pPath.empty())
    {
        return 0.0;
    }

    std::vector<NodeType>::const_iterator i = pPath.begin();
    std::vector<NodeType>::const_iterator lastNode = i;
    ++i;
    double sum = 0.0;
    while(i != pPath.end())
    {
        sum += this->weights.at(std::make_pair(*lastNode, *i));
        lastNode = i;
        ++i;
    }

    return sum;
}