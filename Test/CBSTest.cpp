/**
 * @file CBSTest.cpp
 * @author Lennart Hustermeier (lennart.hustermeier@gmx.de)
 * @brief Contains an implementation of a mini-program which can be used to evaluate the CBS implementation on a 2D grid
 * @version 0.1
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "graph/MAPF/CBS/CBS.hpp"
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

/**
 * @brief Returns a name for a node in a 2D grid as a string
 * 
 * @param pX X-coordinate of the node in the 2D grid
 * @param pY Y-coordinate of the node in the 2D grid
 * @return std::string Name of the node
 */
std::string createNodeName(int pX, int pY)
{
    return std::to_string(pX) + "," + std::to_string(pY);
}

/**
 * @brief Returns the coordinates of a node in a 2D grid by its name (created by createNodeName)
 * 
 * @param pStr The name of the node
 * @return std::pair<unsigned int, unsigned int> a pair (<X>, <Y>) representing the nodes coordinates in the 2D grid 
 */
std::pair<unsigned int, unsigned int> getCoordinatesFromNode(std::string pStr)
{
    size_t pos = pStr.find(',');
    std::string s1 = pStr.substr(0, pos);
    std::string s2 = pStr.substr(pos + 1);
    std::pair<unsigned int, unsigned int> coordinates = std::make_pair(std::stoi(s1), std::stoi(s2));
    return coordinates;
}

/**
 * @brief Creates the 2D grid graph with loops used in this example program
 * 
 * @param pWidth Width of the 2D grid
 * @param pHeight Height of the 2D grid
 * @return Graph The constructed graph
 */
Graph createGraph(unsigned int pWidth, unsigned int pHeight)
{
    std::set<NodeType> nodes;
    std::set<std::tuple<NodeType, NodeType, double>> edges;

    for(unsigned int nodeCntr=0; nodeCntr<pWidth*pHeight; nodeCntr++)
    {
        edges.insert(std::make_tuple(createNodeName(nodeCntr / pWidth, nodeCntr % pWidth), createNodeName(nodeCntr / pWidth, nodeCntr % pWidth), 1.0));
        nodes.insert(createNodeName(nodeCntr / pWidth, nodeCntr % pWidth));
    }
    for(unsigned int nodeCntr=0; nodeCntr<pHeight; nodeCntr++)
    {
        for(unsigned int nodeCntr2=0; nodeCntr2<pWidth-1; nodeCntr2++)
        {
            edges.insert(std::make_tuple(createNodeName(nodeCntr, nodeCntr2), createNodeName(nodeCntr, nodeCntr2 + 1), 1.0));
            edges.insert(std::make_tuple(createNodeName(nodeCntr, nodeCntr2 + 1), createNodeName(nodeCntr, nodeCntr2), 1.0));
        }
    }
    for(unsigned int nodeCntr=0; nodeCntr<pHeight-1; nodeCntr++)
    {
        for(unsigned int nodeCntr2=0; nodeCntr2<pWidth; nodeCntr2++)
        {
            edges.insert(std::make_tuple(createNodeName(nodeCntr, nodeCntr2), createNodeName(nodeCntr + 1, nodeCntr2), 1.0));
            edges.insert(std::make_tuple(createNodeName(nodeCntr + 1, nodeCntr2), createNodeName(nodeCntr, nodeCntr2), 1.0));
        }
    }

    return Graph(nodes, edges);
}

/**
 * @brief Prints the grid, the start and target positions as well as the current positions of the agents
 * 
 * @param pAgentPositions A mapping <agent ID> -> <node> which maps the agents to their current location
 * @param pAgentAssignments A mapping <agent ID> -> (<start>, <target>) which maps the agents to their respective start and target nodes
 * @param pWidth The width of the board (grid graph)
 * @param pHeight The height of the board (grid graph)
 * @param pCollisionCheck true -> Check for node collisions again while printing; false -> Do not check for node collisions while printing
 */
void printBoard(std::map<unsigned int, NodeType> pAgentPositions, std::map<unsigned int, std::pair<NodeType, NodeType>> pAgentAssignments, unsigned int pWidth, unsigned int pHeight, bool pCollisionCheck)
{
    std::map<unsigned int, unsigned int> agentPos;
    std::set<unsigned int> startPos;
    std::map<unsigned int, unsigned int> targetPos;

    for(const auto& p : pAgentPositions)
    {
        std::pair<unsigned int, unsigned int> coordinates = getCoordinatesFromNode(p.second);
        if(pCollisionCheck && agentPos.count(coordinates.second * pWidth + coordinates.first) > 0)
        {
            throw("Collision detected while printing!");
        }
        agentPos[coordinates.second * pWidth + coordinates.first] = p.first;
    }
    for(const auto& p : pAgentAssignments)
    {
        std::pair<unsigned int, unsigned int> coordinates = getCoordinatesFromNode(p.second.first);
        startPos.insert(coordinates.second * pWidth + coordinates.first);
        coordinates = getCoordinatesFromNode(p.second.second);
        targetPos[coordinates.second * pWidth + coordinates.first] = p.first;
    }

    unsigned int cntr;
    for(cntr=0; cntr<pHeight*pWidth; cntr++)
    {
        if(cntr % pWidth == 0)
        {
            std::cout << std::endl;
        }
        if(agentPos.count(cntr) > 0)
        {
            if(targetPos.count(cntr) > 0)
            {
                if(targetPos[cntr] == agentPos[cntr])
                {
                    std::cout << "\x1B[42m";
                }
                else
                {
                    std::cout << "\x1B[43m";
                }
            }
            else if(startPos.count(cntr) > 0)
            {
                std::cout << "\x1B[104m";
            }
            else
            {
                std::cout << "\x1B[107m";
            }
            std::cout << "\x1B[31m" << (char)(agentPos[cntr] + 48) << "\033[0m\x1B[107m \033[0m";
        }
        else if(targetPos.count(cntr) > 0)
        {
            if(agentPos.count(cntr) == 0 || agentPos[cntr] != targetPos[cntr])
            {
                std::cout << "\x1B[43m\x1b[5m" << targetPos[cntr] << "\033[0m\x1B[107m \033[0m";
            }
            else
            {
                std::cout << "\x1B[42m\x1b[5m" << targetPos[cntr] << "\033[0m\x1B[107m \033[0m";
            }
        }
        else if(startPos.count(cntr) > 0)
        {
            std::cout << "\x1B[104m \033[0m\x1B[107m \033[0m";
        }
        else
        {
            std::cout << "\x1B[107m  \033[0m";
        }
    }
    std::cout << std::endl;
    for(const auto& p : pAgentPositions)
    {
        std::cout << "(" << (char)(p.first + 48) << "; " << p.second << ") ";
    }
    std::cout << std::endl;
}
/**
 * @brief Returns a vector of random nodes of a graph
 * 
 * @param pGraph The graph from which to get random nodes
 * @param pNumber The number of random nodes to return
 * @return std::vector<NodeType> pNumber different random nodes from pGraph
 */
std::vector<NodeType> getRandomNodes(Graph& pGraph, unsigned int pNumber)
{
    std::vector<NodeType> nodes;
    std::set<NodeType> n = pGraph.getNodes();
    std::copy(n.begin(), n.end(), std::back_inserter(nodes));
    std::shuffle(nodes.begin(), nodes.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
    return std::vector<NodeType>(nodes.begin(), nodes.begin() + pNumber);
}
/**
 * @brief Extracts program arguments from argv and sets them to default values if not present
 * 
 * @param argc argc
 * @param argv argv
 * @param pSleepTime Time to wait after printing a board (time between steps) 
 * @param pNumAgents The number of agents to create MAPF problems for
 * @param pWidth The width of the grid
 * @param pHeight The height of the grid
 * @param pNumCycles The number of MAPF problems to solve before quitting
 * @param pFilename A filename for a file in which the distribution of the computation time shall be saved
 */
void parseCommandLineArguments(int argc, char* argv[], unsigned int& pSleepTime, unsigned int& pNumAgents, unsigned int& pWidth, unsigned int& pHeight, int& pNumCycles, std::string& pFilename)
{
    pSleepTime = 10000;
    pNumAgents = 10;
    pWidth = 4;
    pHeight = 4;
    pNumCycles = 500;
    pFilename = "distribution.txt";

    if(argc > 1)
    {
        pSleepTime = std::stoi(argv[1]);
    }
    if(argc > 2)
    {
        pNumAgents = std::stoi(argv[2]);
    }
    if(argc > 3)
    {
        pWidth = std::stoi(argv[3]);
    }
    if(argc > 4)
    {
        pHeight = std::stoi(argv[4]);
    }
    if(argc > 5)
    {
        pNumCycles = std::stoi(argv[5]);
    }
    if(argc > 6)
    {
        pFilename = std::string(argv[6]);
    }
}
/**
 * @brief Generates a new random assignment for pNumAgents agents in the graph pGraph
 * 
 * @param pGraph The graph in which the agents shall move
 * @param pNumAgents The number of agents
 * @return std::map<unsigned int, std::pair<NodeType, NodeType>> Mapping <agent ID> -> (<start>, <target>)
 */
std::map<unsigned int, std::pair<NodeType, NodeType>> generateRandomAssignment(Graph& pGraph, unsigned int pNumAgents)
{
    std::vector<NodeType> currentPositionsVector = getRandomNodes(pGraph, pNumAgents);
    std::vector<NodeType> currentTargetsVector = getRandomNodes(pGraph, pNumAgents);

    std::map<unsigned int, std::pair<NodeType, NodeType>> result;

    unsigned int cntr;
    for(cntr=0; cntr<pNumAgents; cntr++)
    {
        result[cntr] = std::make_pair(*(currentPositionsVector.begin()+cntr), *(currentTargetsVector.begin()+cntr));
    }

    return result;
}

/**
 * @brief Main entry point for the test program
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return int Return code
 */
int main(int argc, char* argv[])
{
    unsigned int sleepTime;
    unsigned int numAgents;
    unsigned int width;
    unsigned int height;
    int numCycles;
    std::string filename;

    parseCommandLineArguments(argc, argv, sleepTime, numAgents, width, height, numCycles, filename);
    
    Graph g = createGraph(width, height);

    auto heuristic = [](NodeType p1, NodeType p2) -> double {
        std::pair<unsigned int, unsigned int> coordinates1 = getCoordinatesFromNode(p1);
        std::pair<unsigned int, unsigned int> coordinates2 = getCoordinatesFromNode(p2);

        return std::abs((double)coordinates1.first - coordinates2.first) + std::abs((double)coordinates1.second - coordinates2.second);
    };
    CBS cbsSolver(heuristic);

    std::map<unsigned int, unsigned int> distribution;

    int cycleCntr = 0;
    for(cycleCntr=0; cycleCntr<numCycles && numCycles > 0; cycleCntr++)
    {
        std::map<unsigned int, std::pair<NodeType, NodeType>> assignments = generateRandomAssignment(g, numAgents);
        MAPF::Task t(g, assignments);

        uint64_t startTime = duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

        MAPF::Plan plan = cbsSolver.solveTask(t);

        uint64_t timedif = duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - startTime;
        
        if(!distribution.contains(timedif))
        {
            distribution[timedif] = 1;
        }
        else
        {
            distribution[timedif]++;
        }

        plan.simulate([&](std::map<unsigned int, NodeType> positions) {
            /*Clear the screen*/
            std::cout << "\e[1;1H\e[2J";

            /*Print the current board*/
            printBoard(positions, assignments, width, height, false);
            std::cout << "Calculation time: " << timedif << std::endl;
            for(const auto& d : distribution)
            {
                std::cout << d.first << ":" << d.second << " ";
            }
            std::cout << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        });
    }

    std::ofstream distributionFile(filename);
    for(const auto& d : distribution)
    {
        distributionFile << d.first << ";" << d.second << std::endl;
    }
    distributionFile.flush();
    distributionFile.close();
    return 0;
}