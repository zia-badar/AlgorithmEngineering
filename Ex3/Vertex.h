#ifndef Vertex_h
#define Vertex_h 1

#include <vector>
#include <string>

#define VERTEX_NORMAL 10
#define VERTEX_EXP 11


using namespace std;




class Vertex {
	public:
		Vertex();
		Vertex(int vertex_number);


		string to_string();
		string to_string(int flag);

		int vertex_number; 
		int deg;


	private: 

};

#endif