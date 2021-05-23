#include <string>
#include <vector>
#include "Graph.h"
#include "Vertex.h"
#include "Edge.h"



using namespace std;

Graph parse_input();

int parse_input_vertex_count(string input_line);

vector<int> parse_input_adjacents(string input_line);

bool cluster_detection(Graph graph);

vector<uint> find_p3_not_clique(Graph graph);

tuple<vector<Edge>,int> recursion_main(Graph graph);

tuple<vector<Edge>,bool> recursion(Graph graph,int k);

int connected_component_size(int vertex, Graph graph, int* seen_vertexes);

vector<uint> get_p3_with_bfs(int vertex, Graph graph);