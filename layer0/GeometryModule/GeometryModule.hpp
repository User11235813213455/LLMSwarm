#ifndef GEOMETRY_MODULE_HPP_INCLUDED
#define GEOMETRY_MODULE_HPP_INCLUDED

#include "graph/graph.hpp"
#include "layer0/position.hpp"
#include <algorithm>

class GeometryModule
{
public:
    /**
     * @brief Constructs a new geometry module; Assumption: Two drones are placed on a diagonal of a rectangle which is the base area of a cuboid and all other drones are inside that rectangle. 
     * The environment is the cuboid moved by a pHeightOffset up, with a height pHeight. The nodes are distributed according to the step sizes in pStepSizes (x,y,z, yaw is ignored).
     * 
     * @param pHeightOffset The height offset of the cuboid
     * @param pHeight The height of the cuboid in the same coordinate system from which the drone positions were reported
     * @param pStepSizes The step sizes in x, y and z direction. The yaw is ignored
     * @param pWeights The weights of the x, y and z edges
     * @param pInitialDronePositions The initial positions of the drones
     */
    GeometryModule(double pHeightOffset, double pHeight, Position pStepSizes, Position pWeights, const std::map<uint16_t, Position>& pInitialDronePositions);

    /**
     * @brief Returns the hypercube graph representing the environmnent
     * 
     * @return const Graph& The environment hypercube graph
     */
    const Graph& getEnvironmentGraph() const;

    /**
     * @brief Returns the positions for all nodes of the environment graph
     * 
     * @return const std::map<NodeType, Position> Node -> position mapping
     */
    const std::map<NodeType, Position> getNodePositions() const;

    /**
     * @brief Returns the relative position of a node in the hypercube. This is NOT the real world coordinate tuple
     * 
     * @param pNode The node for which to get the relative position in the hypercube
     * @return std::tuple<uint32_t, uint32_t, uint32_t> A tuple (x,y,z) representing the relative position of the node pNode in the hypercube
     */
    static std::tuple<uint32_t, uint32_t, uint32_t> getNodeHypercubePosition(NodeType pNode);

    /**
     * @brief Snaps a vector of positions to hypercube nodes
     * 
     * @param pPositions Vector of real world positions
     * @return std::vector<NodeType> Nodes on which the positions were snapped (preserves order)
     */
    std::vector<NodeType> snap(const std::vector<Position>& pPositions) const;

    /**
     * @brief Snaps a mapping with positions as values to hypercube nodes
     * 
     * @tparam T The key type of the mapping to be snapped
     * @param pPositionMapping The mapping to be snapped
     * @return std::map<T, NodeType> A mapping which preserved the key values and replaced the values with the nodes which the position was snapped to
     */
    template<class T> std::map<T, NodeType> snap(const std::map<T, Position>& pPositionMapping) const
    {
        std::vector<Position> positions;
        for(const auto& mapping : pPositionMapping)
        {
            positions.push_back(mapping.second);
        }
        std::vector<NodeType> snapPoints = this->snap(positions);
        std::vector<NodeType>::const_iterator c = snapPoints.begin();

        std::map<T, NodeType> result;

        for(const auto& mapping : pPositionMapping)
        {
            result.insert(std::make_pair(mapping.first, *c));
            c++;
        }

        return result;
    }

    /**
     * @brief Translates a node to its real world position
     * 
     * @param pNode The node to get the real world position for
     * @return Position The position of the node in real world coordinates
     */
    Position translateToRealWorldCoordinates(NodeType pNode) const;

    /**
     * @brief Translates a vector of nodes to their real world positions
     * 
     * @param pNodes The nodes to get the real world coordinates for
     * @return std::vector<Position> A vector containing the real world coordinates of pNodes (preserving the order)
     */
    std::vector<Position> translateToRealWorldCoordinates(const std::vector<NodeType>& pNodes) const;

    /**
     * @brief Translates a map to nodes to a map with the same keys with the real world coordinates as values 
     * 
     * @tparam T The key type
     * @param pNodeMapping The mapping to translate
     * @return std::map<T, Position> A mapping T -> Position containing the real world position for the node pNodeMapping[T]
     */
    template<class T> std::map<T, Position> translateToRealWorldCoordinates(const std::map<T, NodeType>& pNodeMapping)
    {
        std::map<T, Position> realWorldCoordinates;
        for(const auto& mapping : pNodeMapping)
        {
            realWorldCoordinates.insert(std::make_pair(mapping.first, this->translateToRealWorldCoordinates(mapping.second)));
        }
        return realWorldCoordinates;
    }

    /**
    * @brief This function takes agent targets and their current position in the graph and tries to move the agents nearer to the
    * target coordinates (intentionally disregarding the graph boundaries) while maintaining the distance given by the step size.
    * The bounding box of the graph can not be left by this method, it's only about providing better accuracy inside of it
    */
    template<class T> std::map<T, Position> refine(const std::map<T, Position>& pTargets, const std::map<T, NodeType>& pPositions)
    {
        double minX = std::min_element(this->nodePositions.begin(), this->nodePositions.end(), [&](const auto& n1, const auto& n2) { return n1.second.getX() < n2.second.getX(); })->second.getX();
        double minY = std::min_element(this->nodePositions.begin(), this->nodePositions.end(), [&](const auto& n1, const auto& n2) { return n1.second.getY() < n2.second.getY(); })->second.getY();
        double minZ = this->minHeight;

        double maxX = std::min_element(this->nodePositions.begin(), this->nodePositions.end(), [&](const auto& n1, const auto& n2) { return n1.second.getX() > n2.second.getX(); })->second.getX();
        double maxY = std::min_element(this->nodePositions.begin(), this->nodePositions.end(), [&](const auto& n1, const auto& n2) { return n1.second.getY() > n2.second.getY(); })->second.getY();
        double maxZ = this->minHeight + this->height;

        double minDistanceXY = std::sqrt(this->stepSizes.getX() * this->stepSizes.getX() / 4 + this->stepSizes.getY() * this->stepSizes.getY() / 4);
        double minDistanceXZ = std::sqrt(this->stepSizes.getX() * this->stepSizes.getX() / 4 + this->stepSizes.getZ() * this->stepSizes.getZ() / 4);
        double minDistanceYZ = std::sqrt(this->stepSizes.getY() * this->stepSizes.getY() / 4 + this->stepSizes.getZ() * this->stepSizes.getZ() / 4);
        double minDistance = std::max<double>({ minDistanceXY, minDistanceXZ, minDistanceYZ });

        std::map<T, Position> result;

        for (const auto& position : pPositions)
        {
            Position target = pTargets.at(position.first);

            /*Clip to graph boundaries*/
            target.setX(std::min<double>(std::max<double>(target.getX(), minX), maxX));
            target.setY(std::min<double>(std::max<double>(target.getY(), minY), maxY));
            target.setZ(std::min<double>(std::max<double>(target.getZ(), minZ), maxZ));

            /*Clip to max movement*/
            Position posRealWorld = this->nodePositions[position.second];
            constexpr double accuracyCorrection = 0.05;
            target.setX(std::min<double>(std::max<double>(target.getX(), posRealWorld.getX() - stepSizes.getX() / 2 - accuracyCorrection), (posRealWorld.getX() + stepSizes.getX() / 2) - accuracyCorrection));
            target.setY(std::min<double>(std::max<double>(target.getY(), posRealWorld.getY() - stepSizes.getY() / 2 - accuracyCorrection), (posRealWorld.getY() + stepSizes.getY() / 2) - accuracyCorrection));
            target.setZ(std::min<double>(std::max<double>(target.getZ(), posRealWorld.getZ() - stepSizes.getZ() / 2 - accuracyCorrection), (posRealWorld.getZ() + stepSizes.getZ() / 2) - accuracyCorrection));

            if (std::none_of(result.begin(), result.end(), [&](const auto& kvp) { return target.getEuclideanDistance(kvp.second) < minDistance; }))
            {
                result[position.first] = target;
            }
            else
            {
                result[position.first] = this->nodePositions.at(position.second);
            }
        }

        return result;
    }

protected:
    /**
     * @brief A hypercube graph representing the environment of the swarm
     */
    Graph environmentGraph;

    /**
     * @brief A mapping node -> position for all graph nodes
     */
    std::map<NodeType, Position> nodePositions;

    /**
     * @brief Generates the name of a node having the coordinates pX, pY and pZ (coordinates are logical/"node" coordinates here)
     * 
     * @param pX The x position of the node in the hypercube (as in pX-th node from x=0)
     * @param pY The y position of the node in the hypercube (as in pY-th node from x=0)
     * @param pZ The z position of the node in the hypercube (as in pZ-th node from x=0)
     * @return std::string The name of the node which will be used in the graph
     */
    static std::string generateNodeName(uint32_t pX, uint32_t pY, uint32_t pZ);

    /**
    * @brief Contains the step sizes in X, Y and Z direction (yaw is ignored)
    */
    Position stepSizes;

    /**
    * @brief Minimum flight height
    */
    double minHeight;

    /**
    * @brief Height (offset to minHeight)
    */
    double height;
};

#endif /*GEOMETRY_MODULE_HPP_INCLUDED*/