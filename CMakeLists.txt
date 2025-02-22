cmake_minimum_required(VERSION 3.10)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Result/OperationController)

set(WINDOWS_BUILD ON)

set(CMAKE_C_CPPCHECK "cppcheck")
set(CMAKE_CXX_CPPCHECK "cppcheck")

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck REQUIRED)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)

project(OperationController)
find_package(Threads)

add_executable(OperationController 
    main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp
)

target_include_directories(OperationController PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(OperationController PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(OperationController PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(OperationController PUBLIC m)
endif()

project(CBSTest)
find_package(Threads)
add_executable(CBSTest graph/MAPF/CBS/CBS.cpp graph/MAPF/CBS/ConstraintTree.cpp graph/MAPF/mapf.cpp graph/graph.cpp Test/CBSTest.cpp logger.cpp)
target_include_directories(CBSTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CBSTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(CBSTest PUBLIC wsock32 ws2_32)
endif()

project(CBSPresentation)
find_package(Threads)
add_executable(CBSPresentation graph/MAPF/CBS/CBS.cpp graph/MAPF/CBS/ConstraintTree.cpp graph/MAPF/mapf.cpp graph/graph.cpp Test/CBSPresentation.cpp logger.cpp)
target_include_directories(CBSPresentation PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CBSPresentation PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(CBSPresentation PUBLIC wsock32 ws2_32)
endif()

project(ProtocolTest)
add_executable(ProtocolTest Test/ProtocolTest.cpp network/protocol.cpp layer0/CommonProtocol.cpp layer0/position.cpp logger.cpp)
target_include_directories(ProtocolTest PUBLIC ${CMAKE_SOURCE_DIR})

if(${WINDOWS_BUILD})
    target_link_libraries(ProtocolTest PUBLIC wsock32 ws2_32)
endif()

project(DroneSwarmInterfaceClientTest)
add_executable(DroneSwarmInterfaceClientTest 
    Test/DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.cpp
    Test/DroneSwarmInterfaceClientTest/main.cpp
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/CommonProtocol.cpp)
if(${WINDOWS_BUILD})
    target_link_libraries(DroneSwarmInterfaceClientTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(DroneSwarmInterfaceClientTest PUBLIC m)
endif()

target_include_directories(DroneSwarmInterfaceClientTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(DroneSwarmInterfaceClientTest PRIVATE Threads::Threads)

project(InteractionInterfaceServerTest)
add_executable(InteractionInterfaceServerTest 
    Test/InteractionInterfaceServerTest/InteractionInterfaceTestClient.cpp
    Test/InteractionInterfaceServerTest/main.cpp
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/position.cpp
    layer0/InteractionInterface/InteractionServer.cpp
    layer0/CommonProtocol.cpp)

target_include_directories(InteractionInterfaceServerTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(InteractionInterfaceServerTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(InteractionInterfaceServerTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(InteractionInterfaceServerTest PUBLIC m)
endif()

project(CombinedTest)
add_executable(CombinedTest 
    Test/InteractionInterfaceServerTest/InteractionInterfaceTestClient.cpp
    Test/DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.cpp
    Test/CombinedTest/main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp)

target_include_directories(CombinedTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CombinedTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(CombinedTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(CombinedTest PUBLIC m)
endif()

project(RealDroneTest)
add_executable(RealDroneTest 
    Test/InteractionInterfaceServerTest/InteractionInterfaceTestClient.cpp
    Test/DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.cpp
    Test/RealDroneTest/main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp)

target_include_directories(RealDroneTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(RealDroneTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(RealDroneTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(RealDroneTest PUBLIC m)
endif()

project(CombinedRealDroneTest)
add_executable(CombinedRealDroneTest 
    Test/InteractionInterfaceServerTest/InteractionInterfaceTestClient.cpp
    Test/CombinedRealDroneTest/main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp)

target_include_directories(CombinedRealDroneTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CombinedRealDroneTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(CombinedRealDroneTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(CombinedRealDroneTest PUBLIC m)
endif()


project(ARTestServer)
add_executable(ARTestServer 
    Test/DroneSwarmInterfaceClientTest/DroneSwarmInterfaceTestServer.cpp
    Test/ARTestServer/main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp)

target_include_directories(ARTestServer PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(ARTestServer PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(ARTestServer PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(ARTestServer PUBLIC m)
endif()

project(CBSSwapTest)
add_executable(CBSSwapTest 
    Test/CBSSwap/main.cpp
    graph/MAPF/CBS/CBS.cpp 
    graph/MAPF/CBS/ConstraintTree.cpp 
    graph/MAPF/mapf.cpp 
    graph/graph.cpp 
    utils.cpp 
    logger.cpp
    network/protocol.cpp 
    network/client.cpp
    network/server.cpp 
    layer0/CommonProtocol.cpp 
    layer0/InteractionInterface/InteractionServer.cpp 
    layer0/position.cpp
    layer0/DroneSwarmInterface/DroneSwarmInterfaceClient.cpp
    layer0/GeometryModule/GeometryModule.cpp
    layer1/SwarmOperationHandler/SwarmOperationHandler.cpp)

target_include_directories(CBSSwapTest PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(CBSSwapTest PRIVATE Threads::Threads)

if(${WINDOWS_BUILD})
    target_link_libraries(CBSSwapTest PUBLIC wsock32 ws2_32)
else()
    target_link_libraries(CBSSwapTest PUBLIC m)
endif()