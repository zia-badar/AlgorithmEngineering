//
// Created by zia on 4/29/21.
//

#ifndef NODE_WEIGHT_PAIR_H
#define NODE_WEIGHT_PAIR_H

class node_weight_pair
{
 public:
	int node_index;
	int weight;

	node_weight_pair() : node_index(-1), weight(0)
	{
	}

	node_weight_pair(int _node_index, int _weight) : node_index(_node_index), weight(_weight)
	{
	}

	friend bool operator<(const node_weight_pair& p1, const node_weight_pair& p2)
	{
		return p1.node_index < p2.node_index;
	}
};

#endif
