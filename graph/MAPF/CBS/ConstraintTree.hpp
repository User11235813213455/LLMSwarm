/**
 * @file ConstraintTree.hpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains declarations of data structures and functions used to manage constraint trees as used in the CBS (collision based search) algorithm
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef CONSTRAINT_TREE_HPP_INCLUDED
#define CONSTRAINT_TREE_HPP_INCLUDED

#include "CBS.hpp"
#include <tuple>
#include <optional>

/**
 * @brief A constraint is a restriction for the pathfinding algorithm: It stores the information that a specific agent is not allowed to enter a specific node
 * at a specific timestep.
 * 
 */
class Constraint
{
public:
    /**
     * @brief Construct a new Step object using a tuple (<timestep>, <agent>, <Node>)
     * 
     */
    Constraint(std::tuple<unsigned int, unsigned int, NodeType>);

    /**
     * @brief Construct a new Step object using a timestep, an agent ID and a node
     * 
     * @param pTimestep The timestep at which to take the step
     * @param pAgent The agent which shall perform the step
     * @param pNode The node which shall be entered by the agent
     */
    Constraint(unsigned int pTimestep, unsigned int pAgent, NodeType pNode);

    bool operator<(const Constraint& pOther) const;

    /**
     * @brief Calculates a hash value for this step
     * 
     * @return size_t Hash value
     */
    size_t hash() const;

    /**
     * @brief Gets the agent which shall perform the step
     * 
     * @return unsigned int ID of the agent
     */
    unsigned int getAgent() const;

    /**
     * @brief Gets the time at which to perform the step
     * 
     * @return unsigned int Timestamp starting at 0
     */
    unsigned int getTimestep() const;

    /**
     * @brief Gets the node which the agent shall move to in this step
     * 
     * @return NodeType Node which the agent shall move to
     */
    NodeType getNode() const;
private:

    /**
     * @brief Stores the step as a tuple representation (<timestep>, <agent>, <node>)
     * 
     */
    std::tuple<unsigned int, unsigned int, NodeType> t;
};

/**
 * @brief A conflict is a tuple (<timestep>, <agent 1>, <agent 2>, <node 1>, <node 2>) and represents a collision between two agents; Ther are two possible kinds of collisions. One are
 * node conflicts. For those the tuple has the form (<timestep>, <agent 1>, <agent 2>, <node>, <node>). Another are edge collisions. These can only happen if two agents try to 
 * swap their nodes and have the form (<timestep>, <agent 1>, <agent 2>, <node which agent 1 wants to enter>, <node which agent 2 wants to enter>).
 */
class Conflict
{
public:
    /**
     * @brief Constructs a new Conflict object using a timestep, two agent IDs and a node
     * 
     * @param pTimestep The timestep at which the conflict occurs
     * @param pAgent1 The first agent which is part of the conflict
     * @param pAgent2 The second agent which is part of the conflict
     * @param pNode1 The node which the first agent entered leading to the conflict
     * @param pNode2 The node which the second agent entered leading to the conflict
     */
    Conflict(unsigned int pTimestep, unsigned int pAgent1, unsigned int pAgent2, NodeType pNode1, NodeType pNode2);

    /**
     * @brief Get the timestep at which the conflict occurs
     * 
     * @return unsigned int Timestep
     */
    unsigned int getTimestep() const;

    /**
     * @brief Get the first agent causing the conflict
     * 
     * @return unsigned int ID of the agent
     */
    unsigned int getAgent1() const;

    /**
     * @brief Get the second agent causing the conflict
     * 
     * @return unsigned int ID of the agent
     */
    unsigned int getAgent2() const;

    /**
     * @brief Get the Node at which the first agent is when the conflict happens
     * 
     * @return NodeType Node at which the first agent is when the conflict occurs
     */
    NodeType getNode1() const;

    /**
     * @brief Get the Node at which the second agent is when the conflict happens
     * 
     * @return NodeType Node at which the second agent is when the conflict occurs
     */
    NodeType getNode2() const;
protected:
    /**
     * @brief Stores the conflict in tuple representation (<timestep>, <agent1>, <agent2>, <node1>, <node2>)
     * 
     */
     std::tuple<unsigned int, unsigned int, unsigned int, NodeType, NodeType> t;
};

/**
 * @brief The ConstraintTree is used by the high level algorithm of CBS (collision based search) and stores constraints as a binary tree:
 * If a collision between two agents occurs, there are two options to prevent it: Either agent 1 shall not be allowed to be on the node which
 * caused the collision or agent 2 isn't allowed to. This "options" are stored in this tree structure.  
 * 
 */
class ConstraintTree
{
public:
    /**
     * @brief Construct a new root for a constraint tree
     * 
     * @param pGraph The graph to be used in the constraint tree
     * @param pAgentTasks The agents and their missions
     */
    ConstraintTree(const Graph& pGraph, const std::map<unsigned int, std::pair<NodeType, NodeType>>& pAgentTasks, std::function<double(NodeType, NodeType)> pH=[](NodeType, NodeType){ return 0.0; });

    /**
     * @brief Construct a new child tree
     * 
     * @param pParent The parent of the tree node
     * @param pAgentConstraint The agent of the additional constraint
     * @param pTimestepConstraint The timestep of the additional constraint
     * @param pNodeConstraint The node of the additional constraint
     */
    ConstraintTree(const ConstraintTree& pParent, Constraint pConstraint, std::function<double(NodeType, NodeType)> pH=[](NodeType, NodeType){ return 0.0; });

    /**
     * @brief Returns the cost sum over all agents
     * 
     * @return double The sum of the path costs
     */
    double getCostSum() const;

    /**
     * @brief Compares two constraint trees (e.g. to create an order in a set that makes sense)
     * 
     * @param pOther The other ConstraintTree to compare against this one
     * @return true The other ConstraintTree has either a bigger cost, a bigger hash value of more or bigger constraints
     * @return false The other ConstraintTree has the same or a lower cost sum, a smaller or equal hash value and less or smaller constraints
     */
    bool operator<(const ConstraintTree& pOther) const;

    /**
     * @brief Returns if there is a solution under the given constraints
     * 
     * @return true There is a solution
     * @return false There is no solution
     */
    bool hasSolution() const;

    /**
     * @brief Finds the first conflict in the solution and returns it (if it exists)
     * 
     * @return std::optional<std::tuple<unsigned int, unsigned int, unsigned int, NodeType>>
     */
    std::optional<Conflict> getFirstConflict() const;

    /**
     * @brief Returns the solution of this ConstraintTree as a vector of Steps
     * 
     * @return std::map<unsigned int, std::map<unsigned int, NodeType>> Represents the solution of this ConstraintTree as mapping time -> agent -> node
     */
    std::map<unsigned int, std::map<unsigned int, NodeType>> getSolution() const;

    /**
     * @brief Returns the hash value of this ConstraintTree
     * 
     * @return size_t Hash value
     */
    size_t getHash() const;

    /**
     * @brief Pretty prints the constraints which are represented by this tree
     * 
     */
    void printConstraints() const;
protected:
    /**
     * @brief This function calculates a has value for the whole ConstraintTree
     * 
     * @return size_t Hash value
     */
    size_t hash() const;

    /**
     * @brief Calculates a completely new solution based on the constraints, the agent tasks and the underlying graph
     * @param pH Heuristic to use for A*
     */
    void calculateSolution(std::function<double(NodeType, NodeType)> pH);
    /**
     * @brief Performs the necessary steps to update the path for a single agent; This can be used if only one constraint was added
     * 
     * @param pAgent The agent to do the recalculation for
     * @param pH Heuristic to use for A*
     */
    void updateSolution(unsigned int pAgent,std::function<double(NodeType, NodeType)> pH);

    /**
     * @brief As the low level pathfinding algorithm returns a path which is only as long as it has to, this functions is used to
     * bring all paths of the agents to the same length (in order to construct a plan).
     */
    void pumpUpSolution();

    /**
     * @brief Returns the constraints specified for the specified agent
     * 
     * @param pAgent The agent for which to get the constraints for
     * @return std::map<unsigned int, std::set<NodeType>> The constraints of the agent
     */
    std::map<unsigned int, std::set<NodeType>> getConstraintsForAgent(unsigned int pAgent) const;

    /**
     * @brief Adds a constraint for an agent
     * 
     * @param pAgent The agent which shall be constrained
     * @param pTimestep The timestep at which the agent pAgent is not allowed on the node pNode
     * @param pNode The node which can not be entered by the agent pAgent on timestep pTimestep
     */
    void addConstraint(Constraint pConstraint);

    /**
     * @brief This function checks a path calculated by the low level path finding algorithm for validity (are all constraints met?)
     * 
     * @param pAgent The agent for which the path was calculated
     * @param pPath The path which was calculated by the low level path finding algorithm
     * @return true The path mets all constraints
     * @return false The path is invalid as at least one constraint is ignored
     */
    bool validateLowLevelPathfinding(unsigned int pAgent, const std::vector<NodeType>& pPath) const;

    /**
     * @brief Maps an Agent ID to a map which in turn maps timesteps to nodes he shall not enter at that timestep 
     * (agent -> timestep -> set of nodes he shall not enter)
     * 
     */
    std::map<unsigned int, std::map<unsigned int, std::set<NodeType>>> constraints;

    /**
     * @brief Stores the solution as a mapping timestep -> agent -> node
     * 
     */
    std::map<unsigned int, std::map<unsigned int, NodeType>> solution;

    /**
     * @brief Stores a mapping, which maps each agent to a pair (<start node>, <target node>)
     * 
     */
    const std::map<unsigned int, std::pair<NodeType, NodeType>>& agentTasks;

    /**
     * @brief The underlying graph of the MAPF problem
     * 
     */
    const Graph& graph;

    /**
     * @brief Maps an agent ID to its path cost using the solution in this ContraintTree
     * 
     */
    std::map<unsigned int, double> costs;

    /**
     * @brief The sum of the costs of all agent paths
     * 
     */
    double costSum;

    /**
     * @brief Stores a hash value to make comparisons faster
     * 
     */
    size_t hashValue;
};

#endif /*CONSTRAINT_TREE_HPP_INCLUDED*/