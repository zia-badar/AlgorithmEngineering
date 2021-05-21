//
// Created by zia on 4/29/21.
//

#ifndef NODE_H
#define NODE_H

#include <set>
#include "node_weight_pair.h"

using namespace std;

class node
{
    public:
        set <node_weight_pair> connected_nodes;
        set <node_weight_pair> disconnected_nodes;

        int composed_node_index_1 = -1;
        int composed_node_index_2 = -1;

        node() {}

        void reset(){
            connected_nodes.clear();
            disconnected_nodes.clear();
            composed_node_index_1 = -1;
            composed_node_index_2 = -1;
        }
};

#endif
