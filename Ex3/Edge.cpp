#include "Edge.h"
#include <vector>

using namespace std;
/**
vertex  1 und 2 
set
*/

Edge::Edge() {
	return;
}

Edge::Edge(Vertex* vertex_1, Vertex* vertex_2, int weight) {
	this->vertex_1 = vertex_1;
	this->vertex_2 = vertex_2;
	this->weight = weight;
	this->forbidden = false;
	this->is_set=false;
	return;
}

string Edge::to_string(int flag) {
	string result = "";
	if (flag == 0) {
		result = std::to_string(weight);
	}
	if (flag == 1) {
		result = "(" + (*(this->vertex_1)).to_string() + "-" + std::to_string(this->weight) + "-" + (*(this->vertex_2)).to_string() + ")";
	}
	return result;
}

string Edge::to_string() {
	return this->to_string(0);
}