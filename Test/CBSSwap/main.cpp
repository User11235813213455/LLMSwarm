#include "graph/MAPF/CBS/CBS.hpp"
#include "graph/MAPF/CBS/ConstraintTree.hpp"
#include "layer0/GeometryModule/GeometryModule.hpp"
#include "logger.hpp"

int main()
{
	std::map<uint16_t, Position> drones = {
		{0, Position(0.0, 0.0, 0.0, 0.0)},
		{1, Position(2.0, 2.0, 0.0, 0.0)},
		{2, Position(1.0, 1.0, 0.0, 0.0)},
		{3, Position(1.0, 2.0, 0.0, 0.0)},
		{4, Position(2.0, 1.0, 0.0, 0.0)}
	};

	/*
	* Original targets: (-0.113100,-0.963900,1.018300,0.000000) (-0.036300,-0.863200,1.030800,0.000000) (-0.064200,-0.929600,1.030800,0.000000) (-0.082100,-0.998500,1.030800,0.000000) (-0.027600,-0.969500,1.030800,0.000000)
	  Original positions: (0.820200,1.599400,1.000000,0.000000) (1.599400,0.779700,1.000000,0.000000) (0.000500,0.020200,1.000000,0.000000) (0.000000,0.820200,1.000000,0.000000) (0.799500,0.780200,1.000000,0.000000)
	*/

	GeometryModule geometry(1.0, 0.5, Position(0.8, 0.8, 0.8, 0.0), Position(0.8, 0.8, 0.8, 0.0), drones);

	std::map<uint16_t, Position> from = {
		{0, Position(0.820200,1.599400,1.000000,0.000000)},
		{1, Position(1.599400,0.779700,1.000000,0.000000)},
		{2, Position(0.000500,0.020200,1.000000,0.000000)},
		{3, Position(0.000000,0.820200,1.000000,0.000000)},
		{4, Position(0.799500,0.780200,1.000000,0.000000)}
	};
	std::map<uint16_t, NodeType> fromSnap = geometry.snap(from);
	
	std::map<uint16_t, Position> to = {
		{0, Position(-0.113100,-0.963900,1.018300,0.000000)},
		{1, Position(-0.036300,-0.863200,1.030800,0.000000)},
		{2, Position(-0.064200,-0.929600,1.030800,0.000000)},
		{3, Position(-0.082100,-0.998500,1.030800,0.000000)},
		{4, Position(-0.027600,-0.969500,1.030800,0.000000)}
	};
	std::map<uint16_t, NodeType> toSnap = geometry.snap(to);

	std::map<unsigned int, std::pair<NodeType, NodeType>> tasks = {
		{0, std::make_pair(fromSnap[0], toSnap[0])},
		{1, std::make_pair(fromSnap[1], toSnap[1])},
		{2, std::make_pair(fromSnap[2], toSnap[2])},
		{3, std::make_pair(fromSnap[3], toSnap[3])},
		{4, std::make_pair(fromSnap[4], toSnap[4])}
	};

	CBS solver([](NodeType pNode1, NodeType pNode2) {
		std::tuple<uint32_t, uint32_t, uint32_t> pos1 = GeometryModule::getNodeHypercubePosition(pNode1);
		std::tuple<uint32_t, uint32_t, uint32_t> pos2 = GeometryModule::getNodeHypercubePosition(pNode2);

		return Position(std::get<0>(pos1),
			std::get<1>(pos1), std::get<2>(pos1), 0.0).getEuclideanDistance(Position(std::get<0>(pos2),
				std::get<1>(pos2), std::get<2>(pos2), 0.0));
	}, 10);

	Graph g = geometry.getEnvironmentGraph();

	MAPF::Task task(g, tasks);
	MAPF::Plan plan = solver.solveTask(task);
	plan.simulate([&](const auto& m) {
		std::string p = "";
		std::map<unsigned int, Position> r = geometry.translateToRealWorldCoordinates<unsigned int>(m);
		for (const auto& x : r)
		{
			p += std::to_string(x.first) + ": (" + m.at(x.first) + ") -> (" + std::to_string(x.second.getX()) + ", " + std::to_string(x.second.getY()) + ") ";
		}
		MSG_INFO(p);
	});
}