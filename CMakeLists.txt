cmake_minimum_required(VERSION 3.10)
project(algeng_wce_solver)


set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Cplex)



add_executable(main include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)



include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})
add_executable(cplex_example ilp_main.cpp)
target_link_libraries(  cplex_example PRIVATE ${CPLEX_LIBRARIES})
target_link_libraries(  cplex_example PRIVATE dl)
