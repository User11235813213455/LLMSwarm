/**
 * @file mapf.hpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains abstractions and declarations used in MAPF (multi-agent path finding) problems
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef MAPF_HPP_INCLUDED
#define MAPF_HPP_INCLUDED

#include <map>
#include "../graph.hpp"

namespace MAPF
{
    /**
     * @brief Abstraction of a MAPF task; Such a task is defined by one or multiple agents which have start and target coordinates and search for a collision free path to their target coordinates
     * 
     */
    class Task
    {
    public:
        /**
         * @brief Constructs a new Task
         * 
         * @param pGraph The graph on which the task shall be solved
         * @param pAgentsStartTarget A map which maps agent IDs to their respective start and target nodes in the Graph pGraph
         */
        Task(Graph& pGraph, const std::map<unsigned int, std::pair<NodeType, NodeType>>& pAgentsStartTarget);

        /**
         * @brief Constructs a new "empty" Task (nothing to be done) based on a Graph
         * 
         * @param pGraph The underlying Graph
         */
        Task(Graph& pGraph);

        /**
         * @brief Adds an agent to a task by giving him an ID and specifying his start and target node.
         * If the agent already exists, it will change the start/target node of the already existing agent.
         * 
         * @param pAgent The ID of the agent to add
         * @param pStart The start node of the agent
         * @param pEnd The target node of the agent
         */
        void addAgent(unsigned int pAgent, NodeType pStart, NodeType pEnd);

        /**
         * @brief Erases an existing agent from the task. If no such agent exists, nothing will be done
         * 
         * @param pAgent The ID of the agent which shall be erased from the task
         */
        void eraseAgent(unsigned int pAgent);

        /**
         * @brief Returns all agents of this task
         * 
         * @return std::set<unsigned int> Set of all agent IDs in this task
         */
        std::set<unsigned int> getAgents() const;

        /**
         * @brief Returns the start node of an agent
         * 
         * @param pAgent ID of the agent
         * @return NodeType Start node of the agent in this task
         */
        NodeType getAgentStart(unsigned int pAgent) const;

        /**
         * @brief Returns the target node of an agent
         * 
         * @param pAgent ID of the agent
         * @return NodeType Target node of the agent in this task
         */
        NodeType getAgentTarget(unsigned int pAgent) const;

        /**
         * @brief Returns a map of all agents with their respective start and target nodes
         * 
         * @return std::map<unsigned int, std::pair<NodeType, NodeType>> Mapping agent ID -> (<start node>, <target node>)
         */
        std::map<unsigned int, std::pair<NodeType, NodeType>> getAgentsStartTarget() const;

        /**
         * @brief Returns a reference to the underlying graph
         * 
         * @return const Graph& The underlying graph
         */
        const Graph& getGraph() const;
    protected:

        /**
         * @brief Stores mapping agent ID -> (<start node>, <target node>) 
         */
        std::map<unsigned int, std::pair<NodeType, NodeType>> agentsStartTarget;

        /**
         * @brief Stores a copy of the underlying graph
         */
        Graph graph;
    };
    /**
     * @brief Abstraction of a MAPF plan; A plan in this sense defines for each timestep for each agent which node he shall enter
     * 
     */
    class Plan
    {
    public:
        /**
         * @brief Constructs a new Plan by defining a mapping <timestep> -> <agent ID> -> <node>
         * 
         * @param pSteps Mapping <timestep> -> <agent ID> -> <node>
         */
        Plan(std::map<unsigned int, std::map<unsigned int, NodeType>> pSteps);

        /**
         * @brief Simulates/Runs a plan by executing its steps and calling a callback function for each individual position assignment
         * 
         * @param pCallback A callback function which will be called for every step of the plan with a mapping <agent ID> -> <node>
         */
        void simulate(std::function<void(const std::map<unsigned int, NodeType>&)> pCallback);
    protected:

        /**
         * @brief Stores the steps of the plan as a mapping <timestep> -> <agent ID> -> <node>
         */
        std::map<unsigned int, std::map<unsigned int, NodeType>> steps;
    };

    /**
     * @brief An abstraction of a MAPF solver; A solver has the ability to find a plan which solves a MAPF task
     * 
     */
    class Solver
    {
    public:
        /**
         * @brief Takes a MAPF task and attempts to find a plan which solves it
         * 
         * @param pTask The task to solve
         * @return Plan The resulting plan
         */
        virtual Plan solveTask(const Task& pTask)=0;
    };
}

#endif /*MAPF_HPP_INCLUDED*/