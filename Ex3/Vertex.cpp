#include "Vertex.h"
#include <vector>
#include <string>

using namespace std;


Vertex::Vertex() {
	return;
}

Vertex::Vertex(int vertex_number) {
	this->vertex_number = vertex_number;
	return;
}

string Vertex::to_string(int flag) {
	if (flag == VERTEX_NORMAL) {
		return "V" + std::to_string(this->vertex_number); 
	}
	if (flag == VERTEX_EXP) {
		return "V" + std::to_string(this->vertex_number) + "(" + std::to_string(this->deg) + ")";
	}
	return this->to_string(VERTEX_NORMAL);
}

string Vertex::to_string() {
	return this->to_string(VERTEX_NORMAL);
}