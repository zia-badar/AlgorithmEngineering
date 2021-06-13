#!/bin/bash

cmake .
#cmake . -DCPLEX_ROOT_DIR=/opt/ibm/ILOG/CPLEX_Studio201/
cmake . -DCPLEX_ROOT_DIR=/home/users/z/zia_badar/CPLEX_Studio201/
make
