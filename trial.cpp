#include "solver.h"
#include <vector>
#include <iostream>
#include <tuple>



int main() {
	
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
