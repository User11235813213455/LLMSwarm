#include "graph/MAPF/CBS/CBS.hpp"
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

int main()
{
    std::set<NodeType> nodes = {"v1", "v2", "v3", "v4", "v5"};
    std::set<std::tuple<NodeType, NodeType, double>> edges = {
        std::make_tuple("v1", "v5", 1.0),
        std::make_tuple("v5", "v1", 1.0),
        std::make_tuple("v2", "v5", 1.0),
        std::make_tuple("v5", "v2", 1.0),
        std::make_tuple("v3", "v5", 1.0),
        std::make_tuple("v5", "v3", 1.0),
        std::make_tuple("v4", "v5", 1.0),
        std::make_tuple("v5", "v4", 1.0),
        std::make_tuple("v1", "v1", 1.0),
        std::make_tuple("v2", "v2", 1.0),
        std::make_tuple("v3", "v3", 1.0),
        std::make_tuple("v4", "v4", 1.0),
        std::make_tuple("v5", "v5", 1.0),
    };

    Graph g(nodes, edges);

    auto heuristic = [](NodeType p1, NodeType p2) -> double {
        return 0.0;
    };

    std::map<unsigned int, std::pair<NodeType, NodeType>> startTarget;
    startTarget[0] = std::make_pair("v1", "v3");
    startTarget[1] = std::make_pair("v2", "v4");
    startTarget[2] = std::make_pair("v3", "v5");

    CBS solver(heuristic);
    MAPF::Task task(g, startTarget);
    solver.solveTask(task).simulate([](auto m) {
        for(auto a : m)
        {
            std::cout << a.first << ": " << a.second << "\t";
        }
        std::cout << std::endl;
    });

    return 0;
}