#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <math.h>
#include <algorithm>

using namespace std;

std::vector<int> parse_input_adjacents(string input_line);
int parse_input_vertex_count(string input_line);
vector<vector<int>> build_adjacent_matrix(int vertex_count, vector<vector<int>> edges);
vector<vector<int>> build_adjacent_combination(unsigned int combination_no, vector<vector<int>> base_matrix, vector<vector<int>> target_vector);
int calculate_change_cost(vector<vector<int>> base_matrix, vector<vector<int>> combination_matrix);
//bool is_cluster(vector<vector<int>> adjacent_matrix);
tuple<bool, vector<vector<int>>> is_cluster(vector<vector<int>> adjacent_matrix);
tuple<bool,vector<vector<int>>> is_cluster_init_new(vector<vector<int>> adjacent_matrix);
tuple<bool,vector<vector<int>>> is_cluster_new(vector<vector<int>> adjacent_matrix);


bool is_connected(vector<vector<int>> adjacent_matrix, int index_1, int index_2);
vector<unsigned int> find_p3_not_clique(vector<vector<int>> adjacent_matrix);
tuple<vector<vector<int>>, bool> recursive_approach(vector<vector<int>> adjacent_matrix, int k);

void print_2D_vector(vector<vector<int>> v);
tuple<vector<vector<int>>,int> recursive_aproach_main(vector<vector<int>> adjacent_matrix);
tuple<vector<vector<int>>,int> naive_aproach_main(vector<vector<int>> adjacent_matrix, int vertex_count);
vector<vector<int>> translate_indices(vector<vector<int>> v);

bool equal_sign(int a, int);
tuple<vector<vector<int>>,vector<int>,int,vector< tuple<vector<int>, vector<int>> >> merge_vertices(vector<vector<int>> adjacent_matrix, int gt_k);
vector<vector<int>> unmerge_vertices(vector<vector<int>> original_matrix, vector<vector<int>> merged_matrix, vector<int> merged_vertices, vector<vector<int>> removed_vertices);

int main(int argc, char **argv) {
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
		std::vector<int> parsed_line = parse_input_adjacents(input_line);
		if (!parsed_line.empty() && parsed_line.size() == 3) {
			adjacent_lines.push_back(parsed_line);
		}
	}
	
	cout << vertex_count << endl;
	cout << adjacent_lines.size() << endl;
	cout << "Building Adjacent-Matrix" << endl;
	vector<vector<int>> adjacent_matrix = build_adjacent_matrix(vertex_count, adjacent_lines);
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = i; j < adjacent_matrix.at(i).size(); j++) {
			adjacent_matrix.at(j).at(i) = adjacent_matrix.at(i).at(j);
		}
	}

	for (int x = 0; x < vertex_count; x++) {
		for (int y = 0; y < vertex_count; y++) {
			cout << adjacent_matrix.at(x).at(y) << " ";
		}
		cout << endl;
	}
	cout << "--------------" << endl;
	

	//tuple<vector<vector<int>>, vector<int>, int> merged = merge_vertices(adjacent_matrix, 3);
	//print_2D_vector(get<0>(merged));
	//tuple<bool, vector<vector<int>>> cluster_detect = is_cluster_init_new(get<0>(merged));
	//cout << get<0>(cluster_detect) << endl;
	//print_2D_vector(get<1>(cluster_detect));
	//cout << "---------------" << endl;


	//vector<vector<int>> unmerged = unmerge_vertices(adjacent_matrix, get<0>(merged), get<1>(merged), get<1>(cluster_detect));
	//print_2D_vector(unmerged);

	tuple<vector<vector<int>>,int> result = recursive_aproach_main(adjacent_matrix);


	//tuple<vector<vector<int>>,int> result = naive_aproach_main(adjacent_matrix, vertex_count);
	
	print_2D_vector(translate_indices(get<0>(result)));
	cout << "cost: " << get<1>(result) << endl;
	return 0;
}

tuple<vector<vector<int>>,int> recursive_aproach_main(vector<vector<int>> adjacent_matrix) {
	vector<int> edge_weights;
	tuple<vector<vector<int>>, bool> result;

	// implement pre-branching reduction rule here
	int k = 0;
	//int l = 0;
	while (true) {
		cout << "test with k= " << k << endl;
		result = recursive_approach(adjacent_matrix, k);
		if (get<1>(result) == true) {
			break;
		}
		k += 1;
		//k += 1;
		//l += 1;
	}
	//cout << "result k: " << k << endl;
	vector<vector<int>> v = get<0>(result);
	//print_2D_vector(v);
	/*for (unsigned int i = 0; i < v.size(); i++) {
		for (unsigned int j = 0; j < v.at(i).size(); j++) {
			cout << v.at(i).at(j) +1 << " ";
		}
		cout << endl;
	}*/
	vector<vector<int>> changed_edges = v;
	int cost = k;
	tuple<vector<vector<int>>, int> end_result(changed_edges, cost);
	return end_result;
}

tuple<vector<vector<int>>,int> naive_aproach_main(vector<vector<int>> adjacent_matrix, int vertex_count) {
	double entry_to_calculate = (double)(pow(((double) vertex_count - 1.),2.) + vertex_count - 1.) / 2.;
	unsigned int combination_count = (unsigned int) pow(2., entry_to_calculate);
	//cout << combination_count << endl;
	vector<vector<int>> target_vector;
	for (int i = 0; i < vertex_count; i++) {
		for (int j = i+1; j < vertex_count; j++) {
			target_vector.push_back({i,j});
		}

	}
	int cost = 100000;
	unsigned int best_combination = 0;
	unsigned int snd_combination = 0;
	int snd_cost = 10000;
	int cluster_counter = 0;
	for (unsigned int i = 1; i <= combination_count; i++) {
		vector<vector<int>> adjacent_matrix_2 = build_adjacent_combination(i, adjacent_matrix, target_vector);
		tuple<bool, vector<vector<int>>> cluster_detected = is_cluster(adjacent_matrix_2);
		if (!get<0>(cluster_detected)) {
			continue;
		}
		cluster_counter += 1;
		int single_cost = calculate_change_cost(adjacent_matrix, adjacent_matrix_2);
		if (single_cost < cost) {
			snd_cost = cost;
			snd_combination = best_combination;
			cost = single_cost;
			best_combination = i;
			continue;
		}
		if (single_cost < snd_cost) {
			snd_cost = single_cost;
			snd_combination = i;
		}

	}
	
	/*for (int x = 0; x < vertex_count; x++) {
		for (int y = 0; y < vertex_count; y++) {
			cout << adjacent_matrix_2.at(x).at(y) << " ";
		}
		cout << endl;
	}*/
	
	cout << "minimal cost: " << cost << endl;
	cout << "2nd cost: " << snd_cost << endl;
	cout << "best combination: " << best_combination << endl;
	cout << "2nd combination: " << snd_combination << endl;
	cout << "clusters detected: " << cluster_counter << endl;
	vector<vector<int>> changed_edges = {};
	tuple<vector<vector<int>>, int> result(changed_edges, cost);
	return result;
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

vector<vector<int>> build_adjacent_matrix(int vertex_count, vector<vector<int>> edges) {
	vector<vector<int>> adjacent_matrix;
	for (int i = 0; i < vertex_count; i++) {
		vector<int> row;
		for (int j = 0; j < vertex_count; j++) {
			row.push_back(0);
		}
		adjacent_matrix.push_back(row);
	}
	//cout << adjacent_matrix.at(0).size() << endl;
	for (unsigned int i = 0; i < edges.size(); i++) {
		adjacent_matrix.at(edges.at(i).at(0)-1).at(edges.at(i).at(1)-1) = edges.at(i).at(2);
	}
	return adjacent_matrix;
}

vector<vector<int>> build_adjacent_combination(unsigned int combination_no, vector<vector<int>> base_matrix, vector<vector<int>> target_vector) {
	unsigned int entry_to_calculate = target_vector.size();
	vector<vector<int>> result_matrix = base_matrix;
	for (unsigned int i = 0; i < entry_to_calculate; i++) {
		unsigned int change = (combination_no >> i) & 1;
		if (change > 0) {
			result_matrix.at(target_vector.at(i).at(0)).at(target_vector.at(i).at(1)) = (result_matrix.at(target_vector.at(i).at(0)).at(target_vector.at(i).at(1))) * -1;
		}
	}
	return result_matrix;
}

int calculate_change_cost(vector<vector<int>> base_matrix, vector<vector<int>> combination_matrix) {
	int cost = 0;
	for (unsigned int i = 0; i < base_matrix.size(); i++) {
		for (unsigned int j = i+1; j < base_matrix.at(i).size(); j++) {
			if (base_matrix.at(i).at(j) == combination_matrix.at(i).at(j)) {
				continue;
			}
			cost += abs(base_matrix.at(i).at(j));
		}
	}
	return cost;
}

void add_to_vector_if_not_exists(vector<vector<int>> &vec_list, vector<int> &vec) {
	if (vec.at(0) > vec.at(1)) {
		int first = vec.at(0);
		vec.at(0) = vec.at(1);
		vec.at(1) = first;
	}
	for (unsigned int i = 0; i < vec_list.size(); i++) {
		if (vec_list.at(i).at(0) == vec.at(0) && vec_list.at(i).at(1) == vec.at(1)) {
			//vec_list.at(i).at(2) = vec.at(2);
			return;
		}
	}
	vec_list.push_back(vec);
	return;
}

tuple<bool,vector<vector<int>>> is_cluster(vector<vector<int>> adjacent_matrix) {
	// fill lower left corner
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = 0; j < adjacent_matrix.at(i).size(); j++) {
			adjacent_matrix.at(j).at(i) = adjacent_matrix.at(i).at(j);
		}
	}
	vector<vector<int>> zero_set;
	//print_2D_vector(adjacent_matrix);
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		vector<int> row = adjacent_matrix.at(i);
		for (unsigned int j = 0; j < row.size(); j++) {
			if (row.at(j) < 0) {
				continue;
			}
			if (row.at(j) == 0 && i != j) {
				//cout << "(" << i << " , " << j << ")" << endl;
				vector<int> zero_edge = {(int) i, (int) j, 0};
				add_to_vector_if_not_exists(zero_set, zero_edge);
				continue;
			}
			for (unsigned int k = j+1; k < row.size(); k++) {
				if (row.at(k) <= 0) {
					continue;
				}
				if (adjacent_matrix.at(j).at(k) < 0) {
					vector<vector<int>> empty_vec;
					tuple<bool,vector<vector<int>>> result(false, empty_vec);
					return result;
				}
				if (adjacent_matrix.at(j).at(k) == 0) {
					//cout << "(" << j << " , " << k << ")" << endl;
					vector<int> zero_edge = {(int) j, (int) k, 1};
					add_to_vector_if_not_exists(zero_set, zero_edge);
					continue;
				} 
			}
		}
	}
	print_2D_vector(zero_set);
	cout << "--------------" << endl;
	tuple<bool,vector<vector<int>>> result(true, zero_set);
	return result;
}

tuple<bool,vector<vector<int>>> is_cluster_init_new(vector<vector<int>> adjacent_matrix) {
	// fill lower left corner
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = 0; j < adjacent_matrix.at(i).size(); j++) {
			adjacent_matrix.at(j).at(i) = adjacent_matrix.at(i).at(j);
		}
	}
	return is_cluster_new(adjacent_matrix);
}

tuple<bool,vector<vector<int>>> is_cluster_new(vector<vector<int>> adjacent_matrix) {
	vector<vector<int>> zero_set;
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		vector<int> row = adjacent_matrix.at(i);
		for (unsigned int j = 0; j < row.size(); j++) {
			if (row.at(j) < 0) {
				continue;
			}
			if (row.at(j) == 0 && i != j) {
				// build new adjacent_matrices (with and without edge)
				// call is_cluster_new
				vector<vector<int>> new_adjacent_matrix_1 = adjacent_matrix;
				vector<vector<int>> new_adjacent_matrix_2 = adjacent_matrix;
				// with edge
				new_adjacent_matrix_1.at(i).at(j) = 1;
				new_adjacent_matrix_1.at(j).at(i) = 1;
				// without edge
				new_adjacent_matrix_2.at(i).at(j) = -1;
				new_adjacent_matrix_2.at(j).at(i) = -1;
				tuple<bool, vector<vector<int>>> with_edge_is_cluster = is_cluster_new(new_adjacent_matrix_1);
				if (get<0>(with_edge_is_cluster)) {
					// concat zero_set + set of with_edge_is_cluster
					vector<int> zero_edge = {(int) i, (int) j, 1};
					add_to_vector_if_not_exists(zero_set, zero_edge);
					for (unsigned int a = 0; a < get<1>(with_edge_is_cluster).size(); a++) {
						add_to_vector_if_not_exists(zero_set, get<1>(with_edge_is_cluster).at(a));
					}
					tuple<bool, vector<vector<int>>> result(true, zero_set);
					return result;
				}

				tuple<bool, vector<vector<int>>> without_edge_is_cluster = is_cluster_new(new_adjacent_matrix_2);
				if (get<0>(without_edge_is_cluster)) {
					// concat zero_set + set of without_edge_is_cluster
					vector<int> zero_edge = {(int) i, (int) j, 0};
					add_to_vector_if_not_exists(zero_set, zero_edge);
					for (unsigned int a = 0; a < get<1>(without_edge_is_cluster).size(); a++) {
						add_to_vector_if_not_exists(zero_set, get<1>(without_edge_is_cluster).at(a));
					}
					tuple<bool, vector<vector<int>>> result(true, zero_set);
					return result;
				}
				continue;
			}
			for (unsigned int k = j+1; k < row.size(); k++) {
				if (row.at(k) <= 0) {
					continue;
				}
				if (adjacent_matrix.at(j).at(k) < 0) {
					vector<vector<int>> empty_vec;
					tuple<bool,vector<vector<int>>> result(false, empty_vec);
					return result;
				}
				if (adjacent_matrix.at(j).at(k) == 0 && j != k) {

					vector<vector<int>> new_adjacent_matrix_1 = adjacent_matrix;
					vector<vector<int>> new_adjacent_matrix_2 = adjacent_matrix;
					// with edge
					new_adjacent_matrix_1.at(j).at(k) = 1;
					new_adjacent_matrix_1.at(k).at(j) = 1;
					// without edge
					new_adjacent_matrix_2.at(j).at(k) = -1;
					new_adjacent_matrix_2.at(k).at(j) = -1;

					tuple<bool, vector<vector<int>>> with_edge_is_cluster = is_cluster_new(new_adjacent_matrix_1);
					if (get<0>(with_edge_is_cluster)) {
						// concat zero_set + set of with_edge_is_cluster
						vector<int> zero_edge = {(int) j, (int) k, 1};
						add_to_vector_if_not_exists(zero_set, zero_edge);
						for (unsigned int a = 0; a < get<1>(with_edge_is_cluster).size(); a++) {
							add_to_vector_if_not_exists(zero_set, get<1>(with_edge_is_cluster).at(a));
						}
						tuple<bool, vector<vector<int>>> result(true, zero_set);
						return result;
					}

					tuple<bool, vector<vector<int>>> without_edge_is_cluster = is_cluster_new(new_adjacent_matrix_2);
					if (get<0>(without_edge_is_cluster)) {
						// concat zero_set + set of without_edge_is_cluster
						vector<int> zero_edge = {(int) j, (int) k, 0};
						add_to_vector_if_not_exists(zero_set, zero_edge);
						for (unsigned int a = 0; a < get<1>(without_edge_is_cluster).size(); a++) {
							add_to_vector_if_not_exists(zero_set, get<1>(without_edge_is_cluster).at(a));
						}
						tuple<bool, vector<vector<int>>> result(true, zero_set);
						return result;
					}
					continue;
				} 
			}
		}
	}
	print_2D_vector(zero_set);
	cout << "--------------" << endl;
	tuple<bool,vector<vector<int>>> result(true, zero_set);
	return result;
}


tuple<vector<vector<int>>, bool> recursive_approach(vector<vector<int>> adjacent_matrix, int k) {
	//cout << "k: " << k << endl;
	if (k < 0) {
		vector<vector<int>> subresult;
		tuple<vector<vector<int>>, bool> result(subresult, false);
		return result;
	}


	tuple<bool, vector<vector<int>>> cluster_detection = is_cluster_init_new(adjacent_matrix);
	//cout << "after cluster detection" << endl;
	//if (is_cluster(adjacent_matrix)) {
	if (get<0>(cluster_detection)) {
		vector<vector<int>> subresult = get<1>(cluster_detection);
		cout << "cluster detected" << endl;
		print_2D_vector(subresult);
		cout << "cluster detected" << endl;
		//vector<vector<int>> unmerged_subresult = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, subresult);
		//for (unsigned int i = 0; i < implicit_changes.size(); i++) {
		//	unmerged_subresult.push_back(implicit_changes.at(i));
		//}

		tuple<vector<vector<int>>, bool> result(subresult, true);
		//tuple<vector<vector<int>>, bool> result(subresult, true);
		return result;
	}





	vector<vector<int>> set_S;
	// merging ------------------------------------------
	vector<vector<int>> saved_adjacent_matrix = adjacent_matrix;
	//tuple<vector<vector<int>>,vector<int>,int> merging = merge_vertices(adjacent_matrix, k);
	tuple<vector<vector<int>>,vector<int>,int, vector<tuple <vector<int>,vector<int>> >> merging = merge_vertices(adjacent_matrix, k);
	
	vector<vector<int>> merged_adjacent_matrix = get<0>(merging);
	vector<int> merged_vertices = get<1>(merging);
	vector<tuple<vector<int>,vector<int>>> implicit_changes = get<3>(merging);

	//cout << "merged: " << merged_vertices.at(0) << " and " << merged_vertices.at(1) << endl;
	print_2D_vector(merged_adjacent_matrix);
	int reduce_k = get<2>(merging);
	if (k >= reduce_k) {
		adjacent_matrix = merged_adjacent_matrix;
		k -= reduce_k;
	}
	else {
		vector<vector<int>> subresult;
		tuple<vector<vector<int>>, bool> result(subresult, false);
		return result;
	}


	// find nodes to merge
	// generate new adjacent matrix
	// save merging --------------------------------------

	//cout << "before cluster detection" << endl;
	

	
	cout << "before find p3" << endl;
	// find P3 in adjacent matrix
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = i; j < adjacent_matrix.at(i).size(); j++) {
			adjacent_matrix.at(j).at(i) = adjacent_matrix.at(i).at(j);
		}
	}

	vector<unsigned int> sub_graph_indices = find_p3_not_clique(adjacent_matrix);
	
	// change one edge
	cout << "after find p3" << endl;
	vector<vector<int>> sub_graph_adjacent_matrix_1 = adjacent_matrix;
	tuple<vector<vector<int>>, bool> subresult_1;
	if (sub_graph_indices.size() > 0) {
		cout << "before recursive 1" << endl;
		cout << sub_graph_indices.at(0) << " - " << sub_graph_indices.at(1) << " - " << sub_graph_indices.at(2) << endl;
		sub_graph_adjacent_matrix_1.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(1)) = sub_graph_adjacent_matrix_1.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(1)) * -1; 
		subresult_1 = recursive_approach(sub_graph_adjacent_matrix_1, k - abs(adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(1))));
	}
	else {
		sub_graph_adjacent_matrix_1 = adjacent_matrix;
		subresult_1 = recursive_approach(sub_graph_adjacent_matrix_1, k);
	}
	
	//tuple<vector<vector<int>>, bool> subresult_1 = recursive_approach(sub_graph_adjacent_matrix_1, k - abs(adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(1))));

	if (get<1>(subresult_1) != false) {
		//set_S = get<0>(subresult_1);
		print_2D_vector(get<0>(subresult_1));
		for (unsigned int i = 0; i < get<0>(subresult_1).size(); i++) {
			set_S.push_back(get<0>(subresult_1).at(i));
		}
		// revert merging
		if (sub_graph_indices.size() > 0) {
			int append = 1;
			if (adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(1)) > 0) {
				append = 0;
			}
			//set_S.push_back({(int) sub_graph_indices.at(0), (int) sub_graph_indices.at(1), append});
		}

		//vector<vector<int>> appending_set = {{(int) sub_graph_indices.at(0), (int) sub_graph_indices.at(1), append}};
		//vector<vector<int>> unmerged_app_set = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, appending_set);

		//for (unsigned int i = 0; i < unmerged_app_set.size(); i++) {
		//	set_S.push_back(unmerged_app_set.at(i));
		//}

		// unmerge with set_S
		cout << "recursive return" << endl;
		print_2D_vector(set_S);
		vector<vector<int>> set_S_unmerged = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, set_S);
		cout << "set_S_unmerged before" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged brefore" << endl;
		for (unsigned int i = 0; i < implicit_changes.size(); i++) {
			bool found = false;
			for (unsigned int j = 0; j < set_S_unmerged.size(); j++) {
				if (get<1>(implicit_changes.at(i)).at(0) == set_S_unmerged.at(j).at(0) &&
					get<1>(implicit_changes.at(i)).at(1) == set_S_unmerged.at(j).at(1)) {
					found = true;
					break;
				}
			}
			if (!found) {
				set_S_unmerged.push_back(get<0>(implicit_changes.at(i)));
			}

			//set_S_unmerged.push_back(implicit_changes.at(i));
		}
		cout << "set_S_unmerged" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged" << endl;
		tuple<vector<vector<int>>,bool> result(set_S_unmerged, true);
		//tuple<vector<vector<int>>,bool> result(set_S, true);
		return result;
	}

	
	// or change other edge
	vector<vector<int>> sub_graph_adjacent_matrix_2 = adjacent_matrix;
	tuple<vector<vector<int>>, bool> subresult_2;
	if (sub_graph_indices.size() > 0) {
		cout << "before recursive II" << endl;
		cout << sub_graph_indices.at(0) << " - " << sub_graph_indices.at(1) << " - " << sub_graph_indices.at(2) << endl;
		sub_graph_adjacent_matrix_2.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2)) = sub_graph_adjacent_matrix_2.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2)) * -1;
		subresult_2 = recursive_approach(sub_graph_adjacent_matrix_2, k - abs(adjacent_matrix.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2))));
	}
	else {
		sub_graph_adjacent_matrix_2 = adjacent_matrix;
		subresult_2 = recursive_approach(sub_graph_adjacent_matrix_2, k);
	}
	//sub_graph_adjacent_matrix_2.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2)) = sub_graph_adjacent_matrix_2.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2)) * -1; 
	//tuple<vector<vector<int>>, bool> subresult_2 = recursive_approach(sub_graph_adjacent_matrix_2, k - abs(adjacent_matrix.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2))));
	if (get<1>(subresult_2) != false) {
		//set_S = get<0>(subresult_2);
		// revert merging
		for (unsigned int i = 0; i < get<0>(subresult_2).size(); i++) {
			set_S.push_back(get<0>(subresult_2).at(i));
		}
		if (sub_graph_indices.size() > 0) {
			int append = 1;
			if (adjacent_matrix.at(sub_graph_indices.at(1)).at(sub_graph_indices.at(2)) > 0) {
				append = 0;
			}
			//set_S.push_back({(int) sub_graph_indices.at(1), (int) sub_graph_indices.at(2), append});
		}
		//vector<vector<int>> appending_set = {{(int) sub_graph_indices.at(1), (int) sub_graph_indices.at(2), append}};
		//vector<vector<int>> unmerged_app_set = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, appending_set);

		//for (unsigned int i = 0; i < unmerged_app_set.size(); i++) {
		//	set_S.push_back(unmerged_app_set.at(i));
		//}

		print_2D_vector(set_S);
		// unmerge with set_S
		cout << "recursive return" << endl;
		vector<vector<int>> set_S_unmerged = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, set_S);
		////for (unsigned int i = 0; i < implicit_changes.size(); i++) {
		////	set_S_unmerged.push_back(implicit_changes.at(i));
		////}
		cout << "set_S_unmerged before" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged brefore" << endl;
		for (unsigned int i = 0; i < implicit_changes.size(); i++) {
			bool found = false;
			for (unsigned int j = 0; j < set_S_unmerged.size(); j++) {
				if (get<1>(implicit_changes.at(i)).at(0) == set_S_unmerged.at(j).at(0) &&
					get<1>(implicit_changes.at(i)).at(1) == set_S_unmerged.at(j).at(1)) {
					found = true;
					
					break;
				}
			}
			if (!found) {
				set_S_unmerged.push_back(get<0>(implicit_changes.at(i)));
			}

			//set_S_unmerged.push_back(implicit_changes.at(i));
		}
		cout << "set_S_unmerged" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged" << endl;
		tuple<vector<vector<int>>,bool> result(set_S_unmerged, true);
		//tuple<vector<vector<int>>,bool> result(set_S, true);
		return result;
	}
	
	// or change thrid edge
	vector<vector<int>> sub_graph_adjacent_matrix_3 = adjacent_matrix;
	tuple<vector<vector<int>>, bool> subresult_3;
	if (sub_graph_indices.size() > 0) {
		cout << "before recursive III" << endl;
		cout << sub_graph_indices.at(0) << " - " << sub_graph_indices.at(1) << " - " << sub_graph_indices.at(2) << endl;
		sub_graph_adjacent_matrix_3.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2)) = sub_graph_adjacent_matrix_3.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2)) * -1; 
		subresult_3 = recursive_approach(sub_graph_adjacent_matrix_3, k - abs(adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2))));
	}
	else {
		sub_graph_adjacent_matrix_1 = adjacent_matrix;
		subresult_3 = recursive_approach(sub_graph_adjacent_matrix_3, k);
	}
	//sub_graph_adjacent_matrix_3.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2)) = sub_graph_adjacent_matrix_3.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2)) * -1; 
	//tuple<vector<vector<int>>, bool> subresult_3 = recursive_approach(sub_graph_adjacent_matrix_3, k - abs(adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2))));
	if (get<1>(subresult_3) != false) {
		//set_S = get<0>(subresult_3);
		for (unsigned int i = 0; i < get<0>(subresult_3).size(); i++) {
			set_S.push_back(get<0>(subresult_3).at(i));
		}
		// revert merging
		if (sub_graph_indices.size() > 0) {
			int append = 1;
			if (adjacent_matrix.at(sub_graph_indices.at(0)).at(sub_graph_indices.at(2)) > 0) {
				append = 0;
			}
			//set_S.push_back({(int) sub_graph_indices.at(0), (int) sub_graph_indices.at(2), append});
		}
		//vector<vector<int>> appending_set = {{(int) sub_graph_indices.at(0), (int) sub_graph_indices.at(2), append}};
		//vector<vector<int>> unmerged_app_set = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, appending_set);

		//for (unsigned int i = 0; i < unmerged_app_set.size(); i++) {
		//	set_S.push_back(unmerged_app_set.at(i));
		//}
		print_2D_vector(set_S);
		cout << "recursive return" << endl;
		vector<vector<int>> set_S_unmerged = unmerge_vertices(saved_adjacent_matrix, adjacent_matrix, merged_vertices, set_S);
		//for (unsigned int i = 0; i < implicit_changes.size(); i++) {
		//	set_S_unmerged.push_back(implicit_changes.at(i));
		//}
		cout << "set_S_unmerged before" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged brefore" << endl;
		for (unsigned int i = 0; i < implicit_changes.size(); i++) {
			bool found = false;
			for (unsigned int j = 0; j < set_S_unmerged.size(); j++) {
				if (get<1>(implicit_changes.at(i)).at(0) == set_S_unmerged.at(j).at(0) &&
					get<1>(implicit_changes.at(i)).at(1) == set_S_unmerged.at(j).at(1)) {
					found = true;
					
					break;
				}
			}
			if (!found) {
				set_S_unmerged.push_back(get<0>(implicit_changes.at(i)));
			}
			//set_S_unmerged.push_back(implicit_changes.at(i));
		}
		cout << "set_S_unmerged" << endl;
		print_2D_vector(set_S_unmerged);
		cout << "/set_S_unmerged" << endl;
		tuple<vector<vector<int>>,bool> result(set_S_unmerged, true);
		//tuple<vector<vector<int>>,bool> result(set_S, true);
		return result;
	}
	
	vector<vector<int>> subresult;
	tuple<vector<vector<int>>,bool> result(subresult, false);
	return result;
}

tuple<vector<vector<int>>, vector<int>, int, vector< tuple<vector<int>,vector<int>> >> merge_vertices(vector<vector<int>> adjacent_matrix, int gt_k) {
	bool found = false;
	unsigned int row = 0;
	unsigned int col = 0;
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = i+1; j < adjacent_matrix.at(i).size(); j++) {
			if (adjacent_matrix.at(i).at(j) > gt_k) {
				row = i;
				col = j;
				cout << "merge_vertices " << row << " " << col << endl;
				found = true;
				break;
			}
		}
		if (found) {
			break;
		}
	}
	int reduce_k = 0;
	vector<int> merged_vertices;
	if (found) {
		// build new adjacent matrix
		vector<vector<int>> new_adjacent_matrix;
		merged_vertices.push_back(row);
		merged_vertices.push_back(col);
		vector<int> removed_row;
		vector<int> removed_col;
		for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
			if (i == col) {
				continue;
			}
			vector<int> new_row;
			for (unsigned int j = 0; j < adjacent_matrix.at(i).size(); j++) {
				if (j == col) {
					removed_col.push_back(adjacent_matrix.at(i).at(j));
					continue;
				}
				if (i >= j) {
					new_row.push_back(0);
					continue;
				}
				new_row.push_back(adjacent_matrix.at(i).at(j));
			}
			new_adjacent_matrix.push_back(new_row);
		}

		vector<tuple< vector<int>,vector<int> >> removed_vertices;

		for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
			if (i == row || i == col) {
				continue;
			}
			if (i < row) {
				// check wether signs different and reduce budget
				if ((adjacent_matrix.at(i).at(row) > 0 && adjacent_matrix.at(i).at(col) < 0) ||
					(adjacent_matrix.at(i).at(row) < 0 && adjacent_matrix.at(i).at(col) > 0)) {
					reduce_k += min(abs(adjacent_matrix.at(i).at(row)), abs(adjacent_matrix.at(i).at(col)));
					// save min edge vertices
					if (abs(adjacent_matrix.at(i).at(row)) < abs(adjacent_matrix.at(i).at(col))) {
						int append = 1;
						if (adjacent_matrix.at(i).at(row) > 0) {
							append = 0;
						}
						vector<int> edge_1 = {(int) i, (int) row, append};
						vector<int> edge_2 = {(int) i, (int) col, 1-append};
						tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
						removed_vertices.push_back(rmv_vert);
						//removed_vertices.push_back({(int) i, (int) row, append});
					}
					else {
						int append = 1;
						if (adjacent_matrix.at(i).at(col) > 0) {
							append = 0;
						}
						vector<int> edge_1 = {(int) i, (int) col, append};
						vector<int> edge_2 = {(int) i, (int) row, 1-append};
						tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
						removed_vertices.push_back(rmv_vert);
						//removed_vertices.push_back({(int) i, (int) col, append});
					}
					/*if ((adjacent_matrix.at(i).at(row) + adjacent_matrix.at(i).at(col)) != 0) {
						cout << "merge_vertices: add sth" << endl;
						if (abs(adjacent_matrix.at(i).at(row)) < abs(adjacent_matrix.at(i).at(col))) {
							if (!equal_sign(adjacent_matrix.at(i).at(row), adjacent_matrix.at(i).at(row) + adjacent_matrix.at(i).at(col))) {
								removed_vertices.push_back({(int) i, (int) row, 1});
							}
						}
						else {
							if (!equal_sign(adjacent_matrix.at(i).at(row), adjacent_matrix.at(i).at(row) + adjacent_matrix.at(i).at(col))) {
								removed_vertices.push_back({(int) i, (int) col, 1});
							}
						}
					}*/

				}
				new_adjacent_matrix.at(i).at(row) = adjacent_matrix.at(i).at(row) + adjacent_matrix.at(i).at(col);
			}
			if (i > row) {
				if (i < col) {
					// check wether signs different and reduce budget
					if ((adjacent_matrix.at(row).at(i) > 0 && adjacent_matrix.at(i).at(col) < 0) ||
						(adjacent_matrix.at(row).at(i) < 0 && adjacent_matrix.at(i).at(col) > 0)) {
						reduce_k += min(abs(adjacent_matrix.at(row).at(i)), abs(adjacent_matrix.at(i).at(col)));
						// save min edge vertices
						if (abs(adjacent_matrix.at(row).at(i)) < abs(adjacent_matrix.at(i).at(col))) {
							int append = 1;
							if (adjacent_matrix.at(row).at(i) > 0) {
								append = 0;
							}
							vector<int> edge_1 = {(int) row, (int) i, append};
							vector<int> edge_2 = {(int) i, (int) col, 1-append};
							tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
							removed_vertices.push_back(rmv_vert);
							//removed_vertices.push_back({(int) row, (int) i, append});
						}
						else {
							int append = 1;
							if (adjacent_matrix.at(i).at(col) > 0) {
								append = 0;
							}
							vector<int> edge_1 = {(int) i, (int) col, append};
							vector<int> edge_2 = {(int) row, (int) i, 1-append};
							tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
							removed_vertices.push_back(rmv_vert);
							//removed_vertices.push_back({(int) i, (int) col, append});
						}
						/*if ((adjacent_matrix.at(row).at(i) + adjacent_matrix.at(i).at(col)) != 0) {
							cout << "merge_vertices: add sth" << endl;
							if (abs(adjacent_matrix.at(row).at(i)) < abs(adjacent_matrix.at(i).at(col))) {
								if (!equal_sign(adjacent_matrix.at(row).at(i), adjacent_matrix.at(row).at(i) + adjacent_matrix.at(i).at(col))) {
									removed_vertices.push_back({(int) row, (int) i, 1});
								}
							}
							else {
								if (!equal_sign(adjacent_matrix.at(row).at(i), adjacent_matrix.at(row).at(i) + adjacent_matrix.at(i).at(col))) {
									removed_vertices.push_back({(int) i, (int) col, 1});
								}
							}
						}*/


					}	
					new_adjacent_matrix.at(row).at(i) = adjacent_matrix.at(row).at(i) + adjacent_matrix.at(i).at(col);
				}
				if (i > col) {
					// TODO: check wether signs different and reduce budget
					if ((adjacent_matrix.at(row).at(i) > 0 && adjacent_matrix.at(col).at(i) < 0) ||
						(adjacent_matrix.at(row).at(i) < 0 && adjacent_matrix.at(col).at(i) > 0)) {
						reduce_k += min(abs(adjacent_matrix.at(row).at(i)), abs(adjacent_matrix.at(col).at(i)));
						// save min edge vertices
						if (abs(adjacent_matrix.at(row).at(i)) < abs(adjacent_matrix.at(col).at(i))) {
							int append = 1;
							if (adjacent_matrix.at(row).at(i) > 0) {
								append = 0;
							}
							
							vector<int> edge_1 = {(int) row, (int) i, append};
							vector<int> edge_2 = {(int) col, (int) i, 1-append};
							tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
							removed_vertices.push_back(rmv_vert);
							//removed_vertices.push_back({(int) row, (int) i, append});
						}
						else {
							int append = 1;
							if (adjacent_matrix.at(col).at(i) > 0) {
								append = 0;
							}
							vector<int> edge_1 = {(int) col, (int) i, append};
							vector<int> edge_2 = {(int) row, (int) i, 1-append};
							tuple<vector<int>,vector<int>> rmv_vert(edge_1, edge_2);
							removed_vertices.push_back(rmv_vert);
							//removed_vertices.push_back({(int) col, (int) i, append});
						}
						/*if ((adjacent_matrix.at(row).at(i) + adjacent_matrix.at(col).at(i)) != 0) {
							cout << "merge_vertices: add sth" << endl;
							if (abs(adjacent_matrix.at(row).at(i)) < abs(adjacent_matrix.at(col).at(i))) {
								if (!equal_sign(adjacent_matrix.at(row).at(i),adjacent_matrix.at(row).at(i) + adjacent_matrix.at(col).at(i))) {
									removed_vertices.push_back({(int) row, (int) i, 1});
								}
							}
							else {
								if (!equal_sign(adjacent_matrix.at(row).at(i),adjacent_matrix.at(row).at(i) + adjacent_matrix.at(col).at(i))) {
									removed_vertices.push_back({(int) col, (int) i, 1});
								}
							}
						}*/
					}
					new_adjacent_matrix.at(row).at(i - 1) = adjacent_matrix.at(row).at(i) + adjacent_matrix.at(col).at(i);
				}
			}
		}
		cout << "merge removed" << endl;
		cout << removed_vertices.size() << endl;
		//print_2D_vector(removed_vertices);
		cout << "/merge removed" << endl;
		vector< tuple<vector<int>,vector<int>> > default_vec;
		tuple< vector<vector<int>>,vector<int>,int, vector< tuple<vector<int>,vector<int>> >> result(new_adjacent_matrix, merged_vertices, reduce_k, default_vec);
		return result;
	}
	vector< tuple<vector<int>,vector<int>> > default_vec;
	tuple< vector<vector<int>>,vector<int>,int, vector< tuple<vector<int>, vector<int>> >> result(adjacent_matrix, merged_vertices, 0, default_vec);
	return result;
}

bool equal_sign(int a, int b) {
	if ((a >= 0 && b >= 0) || (a <= 0 && b <= 0)) {
		return true;
	}
	return false;
}


vector<vector<int>> unmerge_vertices(vector<vector<int>> original_matrix, vector<vector<int>> merged_matrix, vector<int> merged_vertices, vector<vector<int>> removed_vertices) {
	if (merged_vertices.size() <= 0) {
		return removed_vertices;
	}
	cout << "##### unmerge ########" << endl;
	cout << merged_vertices.at(0) << ", " << merged_vertices.at(1) << endl;
	print_2D_vector(removed_vertices);
	cout << "###### /unmerge #######" << endl;
	for (unsigned int i = 0; i < original_matrix.size(); i++) {
		for (unsigned int j = i; j < original_matrix.at(i).size(); j++) {
			original_matrix.at(j).at(i) = original_matrix.at(i).at(j);
		}
	}
	vector<vector<int>> result;
	for (unsigned int i = 0; i < removed_vertices.size(); i++) {
		int removed_edge_v1 = removed_vertices.at(i).at(0);
		int removed_edge_v2 = removed_vertices.at(i).at(1);
		if (removed_edge_v1 == merged_vertices.at(0)) {
			int original_edge_1 = original_matrix.at(removed_edge_v2).at(merged_vertices.at(0));
			int original_edge_2 = original_matrix.at(removed_edge_v2).at(merged_vertices.at(1));
			// rework on original edges
			vector<int> unmerged_edge_1 = {removed_edge_v2, merged_vertices.at(0), 1};
			vector<int> unmerged_edge_2 = {removed_edge_v2, merged_vertices.at(1), 1};
			if (removed_edge_v2 >= merged_vertices.at(1)) {
				original_edge_1 = original_matrix.at(removed_edge_v2+1).at(merged_vertices.at(0));
				original_edge_2 = original_matrix.at(removed_edge_v2+1).at(merged_vertices.at(1));
				unmerged_edge_1.at(0) = unmerged_edge_1.at(0) + 1;
				unmerged_edge_2.at(0) = unmerged_edge_2.at(0) + 1;

			}

			if (!equal_sign(original_edge_1, original_edge_2)) {
				// different sign
				int merged_edge = merged_matrix.at(removed_edge_v1).at(removed_edge_v2);
				if (merged_edge != 0) {
					if (equal_sign(merged_edge, original_edge_1)) {
						// append original edge 1
						// search for edge 2 and remove it

						result.push_back(unmerged_edge_1);
					}
					else {
						// append original edge 2
						// search for edge 1 and remove it
						result.push_back(unmerged_edge_2);
					}
					continue;
				}
				else {
					// merged edge is zero
					if (removed_vertices.at(i).at(2) > 0) {
						// add interpretation
						if (original_edge_1 >= 0) {
							result.push_back(unmerged_edge_2);
							continue;
						}
						result.push_back(unmerged_edge_1);
						continue;
					}
					else {
						// remove interpretation
						if (original_edge_1 >= 0) {
							unmerged_edge_1.at(2) = 0;
							result.push_back(unmerged_edge_1);
							continue;
						}
						unmerged_edge_2.at(2) = 0;
						result.push_back(unmerged_edge_2);
						continue;
					}
				}
			}
			else {
				// equal sign
				result.push_back(unmerged_edge_1);
				result.push_back(unmerged_edge_2);
				continue;
			}
			// ... 
		}


		if (removed_edge_v2 == merged_vertices.at(0)) {
			int original_edge_1 = original_matrix.at(removed_edge_v1).at(merged_vertices.at(0));
			int original_edge_2 = original_matrix.at(removed_edge_v1).at(merged_vertices.at(1));
			// rework on original edges
			vector<int> unmerged_edge_1 = {removed_edge_v1, merged_vertices.at(0), 1};
			vector<int> unmerged_edge_2 = {removed_edge_v1, merged_vertices.at(1), 1};
			if (removed_edge_v1 >= merged_vertices.at(1)) {
				original_edge_1 = original_matrix.at(removed_edge_v1+1).at(merged_vertices.at(0));
				original_edge_2 = original_matrix.at(removed_edge_v1+1).at(merged_vertices.at(1));
				unmerged_edge_1.at(0) = unmerged_edge_1.at(0) + 1;
				unmerged_edge_2.at(0) = unmerged_edge_2.at(0) + 1;
			}
			if (!equal_sign(original_edge_1, original_edge_2)) {
				// different sign
				int merged_edge = merged_matrix.at(removed_edge_v1).at(removed_edge_v2);
				if (merged_edge != 0) {
					if (equal_sign(merged_edge, original_edge_1)) {
						// append original edge 1
						result.push_back(unmerged_edge_1);
					}
					else {
						// append original edge 2
						result.push_back(unmerged_edge_2);
					}
					continue;
				}
				else {
					// merged edge is zero
					if (removed_vertices.at(i).at(2) > 0) {
						// add interpretation
						if (original_edge_1 >= 0) {
							result.push_back(unmerged_edge_2);
							continue;
						}
						result.push_back(unmerged_edge_1);
						continue;
					}
					else {
						// remove interpretation
						if (original_edge_1 >= 0) {
							unmerged_edge_1.at(2) = 0;
							result.push_back(unmerged_edge_1);
							continue;
						}
						unmerged_edge_2.at(2) = 0;
						result.push_back(unmerged_edge_2);
						continue;
					}
				}
			}
			else {
				// equal sign
				result.push_back(unmerged_edge_1);
				result.push_back(unmerged_edge_2);
				continue;
			}
			// ...
		}
		result.push_back(removed_vertices.at(i));
	}
	return result;
}

/*
vector<vector<int>> unmerge_vertices_old(vector<vector<int>> original_matrix, vector<vector<int>> merged_matrix, vector<int> merged_vertices, vector<vector<int>> removed_vertices) {
	vector<vector<int>> result;
	for (unsigned int i = 0; i < removed_vertices.size(); i++) {
		cout << "unmerge " << i << endl; 
		int vertex_1 = removed_vertices.at(i).at(0);
		int vertex_2 = removed_vertices.at(i).at(1);
		cout << "vertex_1: " << vertex_1 << endl;
		cout << "vertex_2: " << vertex_2 << endl;
		if (vertex_1 == merged_vertices.at(0)) {
			int orginal_edge_1 = original_matrix.at(vertex_1).at(merged_vertices.at(0));
			int orginal_edge_2 = original_matrix.at(vertex_1).at(merged_vertices.at(1));
			if ((orginal_edge_1 > 0 && orginal_edge_2 < 0) || 
				(orginal_edge_1 < 0 && orginal_edge_2 > 0)) {
				// different sign
				int merged_edge = merged_matrix.at(vertex_1).at(vertex_2);
				if (merged_edge != 0) {
					// replace removed_vertex by the one with the same sign
					if ((merged_edge > 0 && orginal_edge_1 > 0) || 
						(merged_edge < 0 && orginal_edge_1 < 0)) {
						// TODO: replace by edge_1
						vector<int> unmerged_edge = {vertex_1, merged_vertices.at(0)};
						result.push_back(unmerged_edge);
						continue;

					}
					else {
						// TODO: replace by edge_2
						vector<int> unmerged_edge = {vertex_1, merged_vertices.at(1)};
						result.push_back(unmerged_edge);
						continue;
					}
				}
				else {
					// merged edge is zero
					if (merged_vertices.at(2) > 0) {
						// appended edge interpretation
						if (orginal_edge_1 >= 0) {
							vector<int> unmerged_edge = {vertex_1, merged_vertices.at(0), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
						else {
							vector<int> unmerged_edge = {vertex_1, merged_vertices.at(1), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
					}
					else {
						// removed edge interpretation
						if (orginal_edge_1 >= 0) {
							vector<int> unmerged_edge = {vertex_1, merged_vertices.at(1), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
						else {
							vector<int> unmerged_edge = {vertex_1, merged_vertices.at(0), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}

					}
				}
			}
			else {
				// TODO: append change set for second vertex (both)
				vector<int> unmerged_edge_1 = {vertex_1, merged_vertices.at(0), merged_vertices.at(2)};
				vector<int> unmerged_edge_2 = {vertex_1, merged_vertices.at(1), merged_vertices.at(2)};
				result.push_back(unmerged_edge_1);
				result.push_back(unmerged_edge_2);
				continue;
			}
		}
		if (vertex_2 == merged_vertices.at(0)) {
			int orginal_edge_1 = original_matrix.at(vertex_2).at(merged_vertices.at(0));
			int orginal_edge_2 = original_matrix.at(vertex_2).at(merged_vertices.at(1));
			cout << "original_edge_1: " << orginal_edge_1 << endl;
			cout << "original_edge_2: " << orginal_edge_2 << endl;
			if ((orginal_edge_1 > 0 && orginal_edge_2 < 0) || 
				(orginal_edge_1 < 0 && orginal_edge_2 > 0)) {
				// different sign
				int merged_edge = merged_matrix.at(vertex_1).at(vertex_2);
				cout << "merged_edge " << merged_edge << endl;
				if (merged_edge != 0) {
					// replace removed_vertex by the one with the same sign
					if ((merged_edge > 0 && orginal_edge_1 > 0) || 
						(merged_edge < 0 && orginal_edge_1 < 0)) {
						// TODO: replace by edge_1
						vector<int> unmerged_edge = {vertex_2, merged_vertices.at(0)};
						result.push_back(unmerged_edge);
						continue;

					}
					else {
						// TODO: replace by edge_2
						vector<int> unmerged_edge = {vertex_2, merged_vertices.at(1)};
						result.push_back(unmerged_edge);
						continue;
					}
				}
				else {
					// merged edge is zero
					cout << "merged_edge is zero" << endl;
					if (merged_vertices.at(2) > 0) {
						// appended edge interpretation
						if (orginal_edge_1 >= 0) {
							vector<int> unmerged_edge = {vertex_2, merged_vertices.at(0), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
						else {
							vector<int> unmerged_edge = {vertex_2, merged_vertices.at(1), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
					}
					else {
						// removed edge interpretation
						if (orginal_edge_1 >= 0) {
							vector<int> unmerged_edge = {vertex_2, merged_vertices.at(1), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}
						else {
							vector<int> unmerged_edge = {vertex_2, merged_vertices.at(0), merged_vertices.at(2)};
							result.push_back(unmerged_edge);
							continue;
						}

					}
				}
			}
			else {
				// TODO: append change set for second vertex (both)
				vector<int> unmerged_edge_1 = {vertex_2, merged_vertices.at(0), merged_vertices.at(2)};
				vector<int> unmerged_edge_2 = {vertex_2, merged_vertices.at(1), merged_vertices.at(2)};
				result.push_back(unmerged_edge_1);
				result.push_back(unmerged_edge_2);
				continue;
			}
			continue;
		}
		result.push_back(removed_vertices.at(i));
	}
	return result;
}
*/

vector<unsigned int> find_p3_not_clique(vector<vector<int>> adjacent_matrix) {
	vector<vector<int>> result_p3;
	for (unsigned int i = 0; i < adjacent_matrix.size(); i++) {
		for (unsigned int j = 0; j < adjacent_matrix.size(); j++) {
			for (unsigned int k = 0; k < adjacent_matrix.size(); k++) {
				if (i != k && j != k && i != j) {
					if (adjacent_matrix.at(i).at(j) == 0 || adjacent_matrix.at(j).at(k) == 0 || adjacent_matrix.at(i).at(k) == 0) {
						continue;
					}
					if (is_connected(adjacent_matrix, i, j) && is_connected(adjacent_matrix, j, k)) {
						if (is_connected(adjacent_matrix, i, k) == false) {
							cout << "in find_p3_not_clique" << endl;
							print_2D_vector(adjacent_matrix);

							cout << "/in find_p3_not_clique" << i << " " << j << " " << k << endl;
							vector<unsigned int> result = {i,j,k};
							sort(result.begin(), result.end());
							return result;
						}
					}
				}
			}
		}
	}
	return {};
}

bool is_connected(vector<vector<int>> adjacent_matrix, int index_1, int index_2) {
	
	if (index_1 > index_2) {
		if (adjacent_matrix.at(index_2).at(index_1) > 0) {
			return true;
		}
	}
	if (index_1 < index_2) {
		if (adjacent_matrix.at(index_1).at(index_2) > 0) {
			return true;
		}
	}
	if (index_1 == index_2) {
		return true;
	}
	return false;
}


void print_2D_vector(vector<vector<int>> v) {
	for (unsigned int i = 0; i < v.size(); i++) {
		for (unsigned int j = 0; j < v.at(i).size(); j++) {
			cout << v.at(i).at(j);
			if (j < v.at(i).size()- 1) {
				cout << " ";
			}
		}
		cout << endl;
	}
	return;
}

vector<vector<int>> translate_indices(vector<vector<int>> v) {
	for (unsigned int i = 0; i < v.size(); i++) {
		//for (unsigned int j = 0; j < v.at(i).size(); j++) {
		//	v.at(i).at(j) = v.at(i).at(j) + 1;
		//}
		v.at(i).at(0) = v.at(i).at(0) + 1;
		v.at(i).at(1) = v.at(i).at(1) + 1;
		v.at(i).at(2) = v.at(i).at(2);
	}
	return v;	
}