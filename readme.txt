running "compile.sh" compiles the program
running "benchmark-wce.sh <program-name>" i.e "benchmark-wce.sh ../cplex_with_reduction" script with program name in "wce-students" directory starts testing the program


cplex_with_reduction is cplex with reduction rules
cplex_without_reduction is cplex with out reduction rules
solver_non_optimal is solver with heuristics

*cplex installation on abc01 is causing issues with our program, we have copied our cplex installation to folder /home/users/z/zia_badar/CPLEX_Studio201 on abc01 and verified compile and running, please test on abc01 
*data sets need to copied into "wce-students" directory for "benchmark-wce.sh" script to work
