cmake_minimum_required(VERSION 3.10)
project(algeng_wce_solver)


set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Cplex)
FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )

include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

#add_executable(cplex_example ilp_main.cpp)

#target_link_libraries(  cplex_example PRIVATE ${CPLEX_LIBRARIES})
#target_link_libraries(  cplex_example PRIVATE dl)

link_libraries(${CPLEX_LIBRARIES})
link_libraries(dl)
link_libraries(${Boost_LIBRARIES} )


#add_compile_definitions(SOLVER_OPTIMAL=0 SOLVER_NON_OPTIMAL=0 SOLVER_OPTIMAL_WITH_CPLEX=0 CPLEX_WITH_REDUCTION_RULES=0 CPLEX_WITHOUT_REDUCTION_RULES=0)

add_executable(solver_optimal include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)
target_compile_definitions(solver_optimal PUBLIC SOLVER_OPTIMAL=1 SOLVER_NON_OPTIMAL=0 SOLVER_OPTIMAL_WITH_CPLEX=0 CPLEX_WITH_REDUCTION_RULES=0 CPLEX_WITHOUT_REDUCTION_RULES=0)

add_executable(solver_non_optimal include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)
target_compile_definitions(solver_non_optimal PUBLIC SOLVER_OPTIMAL=0 SOLVER_NON_OPTIMAL=1 SOLVER_OPTIMAL_WITH_CPLEX=0 CPLEX_WITH_REDUCTION_RULES=0 CPLEX_WITHOUT_REDUCTION_RULES=0)

add_executable(solver_optimal_with_cplex include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)
target_compile_definitions(solver_optimal_with_cplex PUBLIC SOLVER_OPTIMAL=0 SOLVER_NON_OPTIMAL=0 SOLVER_OPTIMAL_WITH_CPLEX=1 CPLEX_WITH_REDUCTION_RULES=0 CPLEX_WITHOUT_REDUCTION_RULES=0)

add_executable(cplex_with_reduction include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)
target_compile_definitions(cplex_with_reduction PUBLIC SOLVER_OPTIMAL=0 SOLVER_NON_OPTIMAL=0 SOLVER_OPTIMAL_WITH_CPLEX=0 CPLEX_WITH_REDUCTION_RULES=1 CPLEX_WITHOUT_REDUCTION_RULES=0)

add_executable(cplex_without_reduction include/cluster_graph.h main.cpp include/node.h include/p3.h include/node_weight_pair.h include/verifier.h include/p3s_bucket.h)
target_compile_definitions(cplex_without_reduction PUBLIC SOLVER_OPTIMAL=0 SOLVER_NON_OPTIMAL=0 SOLVER_OPTIMAL_WITH_CPLEX=0 CPLEX_WITH_REDUCTION_RULES=0 CPLEX_WITHOUT_REDUCTION_RULES=1)
