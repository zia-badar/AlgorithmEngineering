#ifndef Graph_h
#define Graph_h 1


#include <vector>
#include <string>
#include "Edge.h"
#include "Vertex.h"


class Graph{
	public:
		Graph();
		
		Graph(int n,vector<vector<int>> list_of_edges);

		void build_adjacent_matrix(int n, vector<vector<int>> list_of_edges);
		
		void add_vertex(int vertex_number);

		void add_vertex(Vertex v);

		string to_string(int flag);
		string to_string();

		int get_next_vertex_number();
		vector<Vertex> vertices; 
		vector<vector<Edge>> adjacent_matrix;
		Graph clone();

	private: 


};

#endif