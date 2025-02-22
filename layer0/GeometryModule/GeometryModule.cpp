#include "GeometryModule.hpp"
#include "layer0/position.hpp"
#include <cmath>
#include <optional>
#include <stdexcept>
#include "logger.hpp"

GeometryModule::GeometryModule(double pHeightOffset, double pHeight, Position pStepSizes, Position pWeights, const std::map<uint16_t, Position>& pInitialDronePositions)
: environmentGraph(), nodePositions(), stepSizes(pStepSizes), minHeight(pHeightOffset), height(pHeight)
{
    std::pair<uint16_t, uint16_t> maxPair;
    double maxDistance = 0.0;

    for(const auto& d : pInitialDronePositions)
    {
        for(const auto& d2 : pInitialDronePositions)
        {
            if(d == d2)
            {
                continue;
            }
            double distance = d.second.getEuclideanDistance(d2.second);
            if(distance > maxDistance)
            {
                maxDistance = distance;
                maxPair = std::make_pair(d.first, d2.first);
            }
        }
    }

    maxDistance = std::sqrt(maxDistance);

    uint32_t cntrX = 0;
    uint32_t cntrY = 0;
    uint32_t cntrZ = 0;

    /*Add nodes*/
    double x;
    double y;
    double z;

    double minY = std::min(pInitialDronePositions.at(maxPair.first).getY(), pInitialDronePositions.at(maxPair.second).getY());
    double maxY = std::max(pInitialDronePositions.at(maxPair.first).getY(), pInitialDronePositions.at(maxPair.second).getY());

    double minX = std::min(pInitialDronePositions.at(maxPair.first).getX(), pInitialDronePositions.at(maxPair.second).getX());
    double maxX = std::max(pInitialDronePositions.at(maxPair.first).getX(), pInitialDronePositions.at(maxPair.second).getX());

    MSG_INFO("Hypercube X-Y-layer edge positions: (x=" + std::to_string(minX) + ", y=" + std::to_string(minY) + "), (x=" + std::to_string(maxX) + ", y=" + std::to_string(maxY) + ") Steps: " + (std::string)pStepSizes);

    cntrY = 0;
    for(y=minY; y <= maxY; y+=pStepSizes.getY())
    {
        cntrX = 0;
        for(x=minX; x <= maxX; x+=pStepSizes.getX())
        {
            NodeType nodeName = this->generateNodeName(cntrX, cntrY, 0);

            this->nodePositions.insert(std::make_pair(nodeName, Position(x, y, minHeight, 0.0)));
            environmentGraph.addNode(nodeName);

            if(cntrX > 0)
            {
                /*There is a node previous (in Y direction) to the current node*/
                NodeType otherNode = GeometryModule::generateNodeName(cntrX-1, cntrY, 0);
                this->environmentGraph.addEdge(otherNode, nodeName, pWeights.getX());
                this->environmentGraph.addEdge(nodeName, otherNode, pWeights.getX());
            }
            if(cntrY > 0)
            {
                /*There is a node previous (in Y direction) to the current node*/
                NodeType otherNode = GeometryModule::generateNodeName(cntrX, cntrY-1, 0);
                this->environmentGraph.addEdge(otherNode, nodeName, pWeights.getY());
                this->environmentGraph.addEdge(nodeName, otherNode, pWeights.getY());
            }
            cntrX++;
        }
        cntrY++;
    }

    if (stepSizes.getZ() <= pHeight)
    {
        /*Create "spikes"*/

        cntrY = 0;
        for (y = minY + pStepSizes.getY() / 2; y < maxY; y += pStepSizes.getY())
        {
            cntrX = 0;
            for (x = minX + pStepSizes.getX() / 2; x < maxX; x += pStepSizes.getX())
            {
                NodeType nodeName = this->generateNodeName(cntrX, cntrY, 1);

                this->nodePositions.insert(std::make_pair(nodeName, Position(x, y, stepSizes.getZ() + minHeight, 0.0)));
                environmentGraph.addNode(nodeName);

                double weight = sqrt(pWeights.getZ() * pWeights.getZ() + pWeights.getX() * pWeights.getX() + pWeights.getY() * pWeights.getY());

                NodeType n1 = GeometryModule::generateNodeName(cntrX, cntrY, 0);
                this->environmentGraph.addEdge(n1, nodeName, weight);
                this->environmentGraph.addEdge(nodeName, n1, weight);

                NodeType n2 = GeometryModule::generateNodeName(cntrX + 1, cntrY, 0);
                this->environmentGraph.addEdge(n2, nodeName, weight);
                this->environmentGraph.addEdge(nodeName, n2, weight);

                NodeType n3 = GeometryModule::generateNodeName(cntrX, cntrY + 1, 0);
                this->environmentGraph.addEdge(n3, nodeName, weight);
                this->environmentGraph.addEdge(nodeName, n3, weight);

                NodeType n4 = GeometryModule::generateNodeName(cntrX + 1, cntrY + 1, 0);
                this->environmentGraph.addEdge(n4, nodeName, weight);
                this->environmentGraph.addEdge(nodeName, n4, weight);

                cntrX++;
            }
            cntrY++;
        }
    }


    MSG_INFO("Number of nodes: " + std::to_string(this->nodePositions.size()));
}

const Graph& GeometryModule::getEnvironmentGraph() const
{
    return this->environmentGraph;
}

const std::map<NodeType, Position> GeometryModule::getNodePositions() const
{
    return this->nodePositions;
}

std::string GeometryModule::generateNodeName(uint32_t pX, uint32_t pY, uint32_t pZ)
{
    return std::to_string(pX) + "," + std::to_string(pY) + "," + std::to_string(pZ);
}

std::tuple<uint32_t, uint32_t, uint32_t> GeometryModule::getNodeHypercubePosition(NodeType pNode)
{
    std::string x;
    std::string y;
    std::string z;

    std::string name = pNode;
    
    std::size_t pos = name.find(',');

    if(pos == std::string::npos || pos + 1 >= name.length())
    {
        throw(std::invalid_argument("Invalid node name passed to GeometryModule::getNodeHypercubePosition()!"));
    }
    
    /*Extract x*/
    x = name.substr(0, pos);

    /*Remove the x component from the string copy*/
    name = name.substr(pos + 1);

    pos = name.find(',');

    if(pos == std::string::npos || pos + 1 >= name.length())
    {
        throw(std::invalid_argument("Invalid node name passed to GeometryModule::getNodeHypercubePosition()!"));
    }
    
    /*Extract y*/
    y = name.substr(0, pos);

    /*Remove the y component from the string copy*/
    name = name.substr(pos + 1);
    
    /*Extract z*/
    z = name.substr(0);

    try
    {
        return std::make_tuple(static_cast<uint32_t>(std::stoul(x)), static_cast<uint32_t>(std::stoul(y)), static_cast<uint32_t>(std::stoul(z)));
    }
    catch(...)
    {
        throw(std::invalid_argument("Invalid node name passed to GeometryModule::getHypercubePosition()!"));
    }
}
std::vector<NodeType> GeometryModule::snap(const std::vector<Position>& pPositions) const
{
    if(pPositions.size() > this->nodePositions.size())
    {
        throw(std::invalid_argument("The positions passed to GeometryModule::snap() could not be snapped due to a lack of available nodes!"));
    }

    std::vector<NodeType> result;
    std::set<NodeType> performedSnaps;

    for(const auto& position : pPositions)
    {
        NodeType bestNode;
        std::optional<double> shortestDistance;

        for(const auto& node : this->nodePositions)
        {
            if(performedSnaps.contains(node.first))
            {
                continue;
            }
            double distance = node.second.getEuclideanDistance(position);
            if(!shortestDistance.has_value() || distance < shortestDistance.value())
            {
                shortestDistance = distance;
                bestNode = node.first;
            }
        }
        result.push_back(bestNode);
        performedSnaps.insert(bestNode);
    }

    return result;
}
std::vector<Position> GeometryModule::translateToRealWorldCoordinates(const std::vector<NodeType>& pNodes) const
{
    std::vector<Position> realWorldCoordinates;
    for(const auto& node : pNodes)
    {
        realWorldCoordinates.push_back(this->nodePositions.at(node));
    }
    return realWorldCoordinates;
}
Position GeometryModule::translateToRealWorldCoordinates(NodeType pNode) const
{
    return this->nodePositions.at(pNode);
}