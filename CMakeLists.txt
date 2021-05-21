cmake_minimum_required(VERSION 3.10)
project(algeng_wce_solver)

set(CMAKE_CXX_STANDARD 14)
add_executable(main include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h)
