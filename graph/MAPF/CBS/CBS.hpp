/**
 * @file CBS.hpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Containts declarations used to solve MAPF problems using CBS (collision based search)
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include "../mapf.hpp"

/**
 * @brief Collision based search (CBS) is a algorithm which solves the MAPF problem by searching for a path for all agents individually
 * from their start node to their target node in a low level algorithm such as A* extended to the time domain, identifying collisions 
 * between them and then creating and managing constraints, which forbid them to cause that collision in a high level algorithm.
 */
class CBS : public MAPF::Solver
{
public:
    /**
     * @brief Creates a new CBS solver
     * 
     * @param pHeuristicLowLevel The heuristic to be used in the low level algorithm for A*; The first argument is the node to evaluate, the second
     * is the target node. Possible heuristics could e.g. be the Manhattan distance or the euclidean distance between the nodes.
     * @param pMaxThreads The maximum number of threads to use to solve the problem
     */
    CBS(std::function<double(NodeType, NodeType)> pHeuristicLowLevel, unsigned int pMaxThreads=24);

    /**
     * @brief Solves a task and returns the plan
     * 
     * @param pTask The task to solve
     * @return MAPF::Plan The plan which solves the task
     */
    MAPF::Plan solveTask(const MAPF::Task& pTask);

    /**
     * @brief Returns the maximum number of threads which will be used to solve MAPF tasks
     * 
     * @return unsigned int The maximum number of threads for this solver
     */
    unsigned int getMaxThreads() const;

    /**
     * @brief Sets the maximum number of threads which will be used to solve MAPF tasks
     * 
     * @param pMaxThreads The maximum number of threads for this solver
     */
    void setMaxThreads(unsigned int pMaxThreads);
protected:

    /**
     * @brief The heuristic for the low level algorithm (A* extended by the time domain)
     */
    std::function<double(NodeType, NodeType)> heuristicLowLevel;

    /**
     * @brief Stores the maximum number of threads to use to solve tasks using this solver
    */
    unsigned int maxThreads;
};