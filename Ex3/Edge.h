#ifndef Edge_h
#define Edge_h 1

#include "Vertex.h"
#include <vector>
#include <string>


#define WEIGHT_ONLY 0
#define TRIPLE 1

class Vertex;

class Edge{
	public:
		Edge();
		Edge(Vertex* vertex_1, Vertex* vertex_2, int weight);
		int weight;
		bool is_set;
		bool forbidden;
		Vertex* vertex_1;
		Vertex* vertex_2;
		string to_string(int flag);
		string to_string();



	private: 
};

#endif