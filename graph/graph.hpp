/**
 * @file graph.hpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains data structures and functions for graph operations (such as shortest path problems, obstacle avoidance, ...)
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <set>
#include <map>
#include <vector>
#include <tuple>
#include <functional>
#include <string>

typedef std::string NodeType;

/**
 * @brief An abstraction of a Graph with a maximum of one edge between a pair of nodes
 */
class Graph
{
public:
    /**
     * @brief Constructs a new graph by a set of nodes and a list of tuples representing the edges in the form (<from>, <to>, <weight>)
     * 
     * @param pNodes The nodes which the graph shall contain
     * @param pEdges The edges of the graph in the form (<from>, <to>, <weight>)
     */
    Graph(std::initializer_list<NodeType> pNodes, std::initializer_list<std::tuple<NodeType, NodeType, double>> pEdges);

    /**
     * @brief Constructs a new graph by a set of nodes and a set of edges as tuples in the form (<from>, <to>, <weight>)
     * 
     * @param pNodes The nodes which the graph shall contain
     * @param pEdges The edges which the graph shall contain as a set of tuples of the form (<from>, <to>, <weight>)
     */
    Graph(std::set<NodeType> pNodes, std::set<std::tuple<NodeType, NodeType, double>> pEdges);

    /**
     * @brief A copy constructor for graphs
     * 
     * @param pOther 
     */
    Graph(const Graph& pOther);

    /**
     * @brief Constructs a new empty graph without nodes or edges
     * 
     */
    Graph();

    /**
     * @brief Destroys a graph
     * 
     */
    ~Graph();

    /**
     * @brief Copy assignment operator
     * 
     * @param pOther Other graph
     * @return Graph& This graph after copying pOther onto it 
     */
    Graph& operator=(Graph& pOther);

    /**
     * @brief Adds a node to the graph
     * 
     * @param pNode The node to add
     * @return true The node was added successfully
     * @return false The node was already present in the graph and thus not added
     */
    bool addNode(NodeType pNode);

    /**
     * @brief Adds an edge to the graph
     * 
     * @param pEdge The edge to add as a pair (<from>, <to>)
     * @param pWeight The weight of the edge
     * @return true The edge was successfully added to the graph
     * @return false The edge could not be added as it was already present
     */
    bool addEdge(std::pair<NodeType, NodeType> pEdge, double pWeight=0.0);

    /**
     * @brief Adds an edge to the graph
     * 
     * @param pFrom The node from which the edge starts
     * @param pTo The node at which the edge ends
     * @param pWeight The weight of the edge
     * @return true The edge was successfully added to the graph
     * @return false The edge could not be added as it was already present
     */
    bool addEdge(NodeType pFrom, NodeType pTo, double pWeight=0.0);

    /**
     * @brief Adds an edge to the graph
     * 
     * @param pEdge The edge to add as a tuple (<from>, <to>, <weight>)
     * @return true The edge was successfully added to the graph
     * @return false The edge could not be added as it was already present
     */
    bool addEdge(std::tuple<NodeType, NodeType, double> pEdge);

    /**
     * @brief Removes a node from the graph
     * 
     * @param pNode The node to remove
     * @return true The node was successfully removed from the graph
     * @return false The node could not be removed as it is not contained in the graph
     */
    bool removeNode(NodeType pNode);

    /**
     * @brief Removes an edge from the graph
     * 
     * @param pEdge A pair representing an edge in the form (<from>, <to>)
     * @return true The edge was successfully deleted from the graph
     * @return false The edge could not be removed as it is not contained in the graph
     */
    bool removeEdge(std::pair<NodeType, NodeType> pEdge);

    /**
     * @brief Returns all outgoing edges of a node
     * 
     * @param pNode The node whose outgoing edges shall be returned
     * @return std::set<NodeType> A set of nodes which can be reached by pNode by a single directly connected edge
     */
    std::set<NodeType> getOutgoingEdges(NodeType pNode) const;

    /**
     * @brief Returns all incoming edges of a node
     * 
     * @param pNode The node whose incoming edges shall be returned
     * @return std::set<NodeType> A set of nodes which reach pNode by a single directly connected edge
     */
    std::set<NodeType> getIncomingEdges(NodeType pNode) const;

    /**
     * @brief Returns a set of all nodes in the graph
     * 
     * @return std::set<NodeType> A set containing all graph nodes
     */
    std::set<NodeType> getNodes() const;

    /**
     * @brief Returns a mapping which maps all graph nodes to a vector of nodes which form the shortest way from a start node pStart to them while avoiding obstacles 
     * 
     * @param pStart The start node of the ways
     * @param pObstacles A set of static obstacles which are on nodes which an agent can not enter
     * @return std::map<NodeType, std::vector<NodeType>> A mapping which maps every node to a path over which to reach the node on the fastest way from pStart while avoiding the obstacles of pObstacles 
     */
    std::map<NodeType, std::vector<NodeType>> getAllShortestPaths(NodeType pStart, std::set<NodeType> pObstacles=std::set<NodeType>()) const;

    /**
     * @brief Returns a mapping which maps all graph nodes to a vector of nodes which form the shortest way from a start node pStart to them  and the costs of the path while avoiding obstacles 
     * 
     * @param pStart The start node of the ways
     * @param pObstacles A set of static obstacles which are on nodes which an agent can not enter
     * @return std::map<NodeType, std::pair<std::vector<NodeType>, double>> A mapping which maps every node to a path over which to reach the node on the fastest way from pStart while avoiding the obstacles of pObstacles 
     */
    std::map<NodeType, std::pair<std::vector<NodeType>, double>> getAllShortestPathsWithCosts(NodeType pStart, std::set<NodeType> pObstacles=std::set<NodeType>()) const;
    
    /**
     * @brief Returns a shortest path between the start node pStart and a target node pTarget using the heuristic pH for A*, a set of obstacles which can not be entered by an agent and constraints which forbid entering nodes at specific
     * time steps
     * 
     * @param pStart The start node
     * @param pTarget The target node
     * @param pH A heuristic to use for A*
     * @param pObstacles Static obstacles on nodes which shall not be entered
     * @param pConstraints Mappings <timestep> -> <set of nodes> which specify which nodes shall not be entered at a specific timestep
     * @return std::vector<NodeType> A vector which contains the node of the shortest path
     */
    std::vector<NodeType> getShortestPath(NodeType pStart, NodeType pTarget, std::function<double(NodeType, NodeType)> pH=[](NodeType, NodeType){ return 0.0; }, std::set<NodeType> pObstacles=std::set<NodeType>(), std::map<unsigned int, std::set<NodeType>> pConstraints=std::map<unsigned int, std::set<NodeType>>()) const;
    
    /**
     * @brief Returns the costs of a path in this graph
     * 
     * @param pPath The path to calculate the costs for
     * @return double The costs of this path
     */
    double getPathCost(const std::vector<NodeType>& pPath) const;

    /**
     * @brief Returns the weight of a specific edge in this graph
     * 
     * @param pEdge The edge to get the weight for in the form (<start node>, <end node>)
     * @return double The weight of the edge
     */
    double getWeight(std::pair<NodeType, NodeType> pEdge) const;

    /**
     * @brief Returns the weight of a specific edge in this graph
     * 
     * @param pStart The start node of the edge
     * @param pEnd The end node of the edge
     * @return double The weight of the edge
     */
    double getWeight(NodeType pStart, NodeType pEnd) const;

    /**
     * @brief Returns all outgoing edges of a node and their respective weights
     * 
     * @param pNode The node to get the outgoing edges and their weights for
     * @return std::map<NodeType, double> A mapping <node> -> <weight> for all nodes directly connected to pNode by an outgoing edge
     */
    std::map<NodeType, double> getOutgoingEdgesWithWeights(NodeType pNode) const;

    /**
     * @brief Returns all incoming edges of a node and their respective weights
     * 
     * @param pNode The node to get the incoming edges and their weights for
     * @return std::map<NodeType, double> A mapping <node> -> <weight> for all nodes directly connected to pNode by an incoming edge
     */
    std::map<NodeType, double> getIncomingEdgesWithWeights(NodeType pNode) const;

    /**
     * @brief Generates a new node with a random identifier which is not already present in the graph; Does NOT add the node to the graph,
     * just calculates a possible name
     * 
     * @return NodeType A possible name for a new node
     */
    NodeType generateNewNode() const;

    /**
     * @brief Checks if a path satisfies specific constraints
     * 
     * @param pPath The path to check as a vector of nodes
     * @param pConstraints The constraints as a mapping <timestep> -> <set of nodes which shall not be entered>
     * @return true The path pPath satisfies all constraints pConstraints
     * @return false The path pPath does not satisfy at least one constraint of pConstraints
     */
    bool checkPathConstraints(const std::vector<NodeType>& pPath, const std::map<unsigned int, std::set<NodeType>> pConstraints) const;
protected:
    /**
     * @brief A set storing the nodes of the graph
     */
    std::set<NodeType> nodes;

    /**
     * @brief A mapping <from> -> <to (set)> which stores the outgoing edges in the graph
     */
    std::map<NodeType, std::set<NodeType>> edges;

    /**
     * @brief A mapping <from> -> <to (set)> which stores the incoming edges in the graph
     */
    std::map<NodeType, std::set<NodeType>> edgesIn;

    /**
     * @brief A mapping (<from >, <to>) -> <weight> which stores all edge weights of the graph
     */
    std::map<std::pair<NodeType, NodeType>, double> weights;
};