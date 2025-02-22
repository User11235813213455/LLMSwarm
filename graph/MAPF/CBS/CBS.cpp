/**
 * @file CBS.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Implementations of functions to perform CBS (collision based search)
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "CBS.hpp"
#include "ConstraintTree.hpp"
#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <array>

CBS::CBS(std::function<double(NodeType, NodeType)> pHeuristicLowLevel, unsigned int pMaxThreads)
: heuristicLowLevel(pHeuristicLowLevel), maxThreads(pMaxThreads)
{

}
unsigned int CBS::getMaxThreads() const
{
    return this->maxThreads;
}
void CBS::setMaxThreads(unsigned int pMaxThreads)
{
    this->maxThreads = pMaxThreads;
}

/**
 * @brief Sets the maximum number of threads which will be used to solve MAPF tasks
 * 
 * @param pMaxThreads The maximum number of threads for this solver
 */
void setMaxThreads(unsigned int pMaxThreads);
MAPF::Plan CBS::solveTask(const MAPF::Task& pTask)
{
    std::map<unsigned int, std::pair<NodeType, NodeType>> startTarget = pTask.getAgentsStartTarget();

    /*Construct root node*/
    ConstraintTree R(pTask.getGraph(), startTarget, this->heuristicLowLevel);

    std::set<ConstraintTree> open;

    std::array<const ConstraintTree*, 24> processingData;
    std::array<ConstraintTree*, 48> processingResult;
    std::vector<std::thread> threads;

    /*Also save a closed list; This one is used to prevent searching the same solution twice and saves the hash
    values of all erased tree nodes*/
    std::set<unsigned int> closed;

    /*Add the root node to the open list*/
    open.insert(R);
    while(!open.empty())
    {
        threads.clear();

        /*Calculate how many threads to spawn; There shall be no more tasks than nodes in the open list*/
        unsigned int numThreads = std::min(open.size(), (size_t)24);

        unsigned int threadCntr = 0;
        for(threadCntr=0; threadCntr<numThreads; threadCntr++)
        {
            /*Create tasks that calculate a solution for the nth best element in the open list and searches for collisions;
            The input data for the tasks is stored in the processingData array, the output is stored in the processingResult
            array*/
            const ConstraintTree& P = *std::next(open.begin(), threadCntr);
            processingData[threadCntr] = &P;
            threads.emplace_back([threadCntr, &processingData, &processingResult, this]() {
                /*Search for first conflict in the current nodes solution*/
                std::optional<Conflict> C = processingData[threadCntr]->getFirstConflict();

                processingResult[threadCntr * 2] = nullptr;
                processingResult[threadCntr * 2 + 1] = nullptr;

                if(!C.has_value())
                {
                    /*No conflict -> optimal path found*/
                    return;
                }
                Conflict conflict = C.value();

                /*Calculate solutions for the two different possible constraints due to the previously found conflict*/
                ConstraintTree* child1 = new ConstraintTree(*processingData[threadCntr], Constraint(conflict.getTimestep(), conflict.getAgent1(), conflict.getNode1()), this->heuristicLowLevel);
                ConstraintTree* child2 = new ConstraintTree(*processingData[threadCntr], Constraint(conflict.getTimestep(), conflict.getAgent2(), conflict.getNode2()), this->heuristicLowLevel);

                if(child1->hasSolution())
                {
                    processingResult[threadCntr * 2] = child1;
                }
                if(child2->hasSolution())
                {
                    processingResult[threadCntr * 2 + 1] = child2;
                }
                return;
            });
        }

        /*Wait for threads*/
        for(auto& t : threads)
        {
            
            t.join();
        }
        
        /*Use an optional as we have to iterate over all threads anyway in order to delete their memory and prevent a leak*/
        std::optional<std::map<unsigned int, std::map<unsigned int, NodeType>>> solution;
        double minCostSum = DBL_MAX;
        for(threadCntr=0; threadCntr<numThreads; threadCntr++)
        {
            if(processingResult[threadCntr * 2] == nullptr && processingResult[threadCntr * 2 + 1] == nullptr && !processingData[threadCntr]->getFirstConflict().has_value())
            {
                /*No child -> no conflict -> solution*/
                if (!solution.has_value())
                {
                    solution = processingData[threadCntr]->getSolution();
                }
                else
                {
                    if (processingData[threadCntr]->getCostSum() <= minCostSum)
                    {
                        solution = processingData[threadCntr]->getSolution();
                    }
                }
            }
            else
            {
                if(processingResult[threadCntr * 2] != nullptr)
                {
                    if(!closed.contains(processingResult[threadCntr * 2]->getHash()))
                    {
                        open.insert(*processingResult[threadCntr * 2]);
                    }
                    delete processingResult[threadCntr * 2];
                }
                if(processingResult[threadCntr * 2 + 1] != nullptr)
                {
                    if(!closed.contains(processingResult[threadCntr * 2 + 1]->getHash()))
                    {
                        open.insert(*processingResult[threadCntr * 2 + 1]);
                    }
                    delete processingResult[threadCntr * 2 + 1];
                }
            }
        }
        if(solution.has_value())
        {
            /*We found a solution and cleared the threads memory up -> return*/
            return MAPF::Plan(solution.value());
        }
        for(threadCntr=0; threadCntr<numThreads; threadCntr++)
        {
            closed.insert(processingData[threadCntr]->getHash());
            open.erase(*processingData[threadCntr]);
        }
    }

    return MAPF::Plan(std::map<unsigned int, std::map<unsigned int, NodeType>>());
}