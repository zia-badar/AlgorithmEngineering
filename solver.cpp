#include "solver.h"
#include <vector>
#include <iostream>
#include <tuple>



int main() {
	
	//Edge test_edge(1,2,5);
	//cout << test_edge.to_string() << endl;
	Graph g = parse_input();
	cout << g.to_string() << endl;
	int s[1] = {};
	cout << s[0] << endl;
	//int* seen_vertexes[2] = {};
	int* seen_vertexes = new int[g.vertices.size()]();
	
	cout << seen_vertexes[0] << endl;
	vector<uint> p3 = find_p3_not_clique(g);	
	for(uint i = 0; i < p3.size(); i++){
		cout << p3.at(i) << " ";
	}
	cout << endl;




	return 0;
}

tuple<vector<Edge>,int> recursion_main(Graph graph){
	int k = 0;

	tuple<vector<Edge>,bool> result;

	while (true) {
		cout << "test with k= " << k << endl;
		result = recursion(graph, k);
		if (get<1>(result) == true) {
			break;
		}
		k += 1;
	
	}
	tuple<vector<Edge>,int> solution(get<0>(result),k);

	return solution;
}

tuple<vector<Edge>,bool> recursion(Graph graph,int k){
	if (k < 0) {
		vector<Edge> subresult;
		tuple<vector<Edge>, bool> result(subresult, false);
		return result;
	}

	
	bool is_cluster = cluster_detection(graph);
	if(is_cluster){
		vector<Edge> edges;
		tuple<vector<Edge>,bool> result(edges,true);
		return result;
	}

	// -------------------- find p3 not clique in graph -------------------------
	

	// ------------- recursive call with first edge changed ---------------------
	//	-> clone actual graph
	Graph sub_graph_1 = graph.clone();
	//	-> change the edge if not forbidden (is_set = !is_set)
	//	-> make recursive call with decreased budget
	//	-> check result and if this branch is a solution
	//		-> add the changed edge to the result
	//		-> return the result

	// ------------- recursive call with second edge changed -------------------
	//	-> clone actual graph
	Graph sub_graph_2 = graph.clone();
	//	-> change the edge if not forbidden (is_set = !is_set)
	//	-> make recursive call with decreased budget
	//	-> check result and if this branch is a solution
	//		-> add the changed edge to the result
	//		-> return the result

	// ------------- recursive call with third edge changed --------------------
	//	-> clone actual graph
	Graph sub_graph_3 = graph.clone();
	//	-> change the edge if not forbidden (is_set = !is_set)
	//	-> make recursive call with decreased budget
	//	-> check result and if this branch is a solution
	//		-> add the changed edge to the result
	//		-> return the result



	vector<Edge> empty_subresult;
	tuple<vector<Edge>,bool> result(empty_subresult, false);
	return result;
}

bool cluster_detection(Graph graph){
	for(uint i = 0; i < graph.adjacent_matrix.size(); i++){
		vector<Edge> row = graph.adjacent_matrix.at(i);
		for(uint j = 0; j < row.size();j++){
			if(!row.at(j).is_set){
				continue;
			}
			for(uint k = j+1;k < row.size(); k++){
				if(!row.at(k).is_set){
					continue;
				}
				if (!graph.adjacent_matrix.at(j).at(k).is_set) {
					return false;
				}
			}
		}
	}
	return true;
}

vector<uint> find_p3_not_clique(Graph graph) {

	for (uint i = 0; i < graph.vertices.size(); i++) {
		uint array_length =  graph.vertices.size();
		int* seen_vertexes = new int[array_length]();
		int cc_size = connected_component_size(i, graph, seen_vertexes);
		cout << cc_size << " " << i << " " << graph.vertices.at(i).deg << endl;
		if(graph.vertices.at(i).deg != (cc_size - 1 )){
			return get_p3_with_bfs(i,graph);
		}
	}	
	return {};
}

vector<uint> get_p3_with_bfs(int vertex, Graph graph){
	vector<Edge> row = graph.adjacent_matrix.at(vertex);
	uint not_connected;
	for(uint i = 0;i <  row.size(); i++ ){
		if((i != (uint) vertex) && !row.at(i).is_set ){
			not_connected = i;
			
			break;
		}
	}
	cout << not_connected << endl;
	for(uint i = 0; i < graph.vertices.size(); i++ ){
		cout << graph.adjacent_matrix.at(not_connected).at(i).is_set << " " << graph.adjacent_matrix.at(vertex).at(i).is_set << endl;
		if(graph.adjacent_matrix.at(not_connected).at(i).is_set && graph.adjacent_matrix.at(vertex).at(i).is_set){
			cout << i << endl;
			vector<uint> result = {(uint) vertex,not_connected,i};
			return result;
		}
	}
	vector<uint> result;
	return result;
}




int connected_component_size(int vertex, Graph graph, int* seen_vertexes){
	if(seen_vertexes[vertex]!=0){
		return 0;
	}
	seen_vertexes[vertex]= 1; 
	int result = 1; 

	vector<Edge> row = graph.adjacent_matrix.at(vertex);
	for(uint i = 0; i < row.size();i++){
		if(row.at(i).is_set && ((uint) vertex != i) && seen_vertexes[i]==0){
			result += connected_component_size(i,graph,seen_vertexes);
		}
	}
	return result;
}





Graph parse_input() {
	string input_line;
	int i = 1;
	int vertex_count = 0;
	vector<vector<int>> adjacent_lines;
	while (getline(cin, input_line)) {
		if (i == 1) {
			// parse vertex count
			vertex_count = parse_input_vertex_count(input_line);
			if (vertex_count > 0) {
				i += 1;
			}
			continue;
		}
		vector<int> parsed_line = parse_input_adjacents(input_line);
		if (!parsed_line.empty() && parsed_line.size() == 3) {
			// graph add edge and/or vertex
			adjacent_lines.push_back(parsed_line);
		}
	}
	Graph graph(vertex_count, adjacent_lines);
	return graph;
}

int parse_input_vertex_count(string input_line) {
	if (input_line.empty()) {
		return 0;
	}
	int count = stoi(input_line);
	return count;
}

std::vector<int> parse_input_adjacents(string input_line) {
	if (input_line.empty()) {
		return {};
	}
	std::vector<int> edge_tokens;
	int token;
	size_t position = 0;
	size_t comment_position = input_line.find("#");
	if (comment_position != string::npos) {
		input_line = input_line.substr(0, comment_position);
	}
	while (position != string::npos) {
		position = input_line.find(" ");
		if (!input_line.substr(0, position).empty()) {
			token = stoi(input_line.substr(0, position));
			edge_tokens.push_back(token);
		}
		input_line.erase(0, position + 1);
	}
	return edge_tokens;
}