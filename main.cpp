//
// Created by zia on 4/21/21.
//

#include <iostream>
#include <ctime>
#include <string>
#include "include/cluster_graph.h"
#include "include/verifier.h"
#include "include/processor.h"

using namespace std;

//cluster_graph cg;

// O(3^log(k)*n*log(n) + log(k)*n^(2))  --  log(k) is cost of binary search, 3^log(k)*n*log(n) total cost of cg.solve, log(k)*n^2 is total cost of resetting cluster graph
//int binary_search_for_optimal_k(int l, int r)
//{ // r is always considered to be the solution, just not always the optimal
//	if (l == r)
//		return r;
//
//	int m = (l + r) / 2;
//	int ret = cg.solve(m);
//	if (ret == -1)
//		return binary_search_for_optimal_k(m + 1, r);
//	else
//	{
//		cg.reset_graph(); // resetting graph to starting position after finding solution
//		return binary_search_for_optimal_k(l, m);
//	}
//}

void create_random_input_file(int n, int r)
{
	ofstream fout("random");
	fout << n << endl;
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
			fout << i + 1 << " " << j + 1 << " " << (rand() % 2 == 1 ? -1 : 1) * (rand() % r + 1) << endl;
	fout.close();
}

void print_vec(vector<string> permutation)
{
	for (int i = 0; i < permutation.size(); i++)
		cout << permutation[i] << ", ";
	cout << endl;
}

void permutations(int r,
	vector<string> values,
	vector<string> permutation,
	set<vector<string>>& result_permutations,
	bool repeatition_allowed = false)
{
	if (r < 0)
	{
		result_permutations.insert(vector<string>(permutation));
		return;
	}

	string tmp = "";
	for (int i = 0; i < values.size(); i++)
		if (repeatition_allowed || values[i].compare("") != 0)
		{
			permutation[r] = values[i];
			if (!repeatition_allowed)
			{
				tmp = values[i];
				values[i] = "";
			}
			permutations(r - 1, values, permutation, result_permutations);
			if (!repeatition_allowed)
				values[i] = tmp;
			permutation[r] = "";
		}
}

vector<vector<string>> cartesian_product(set<vector<string>> set_1, set<vector<string>> set_2)
{
	vector<vector<string>> result;
	vector<string> vec_i, vec_j;
	for (auto i = set_1.begin(); i != set_1.end(); i++)
		for (auto j = set_2.begin(); j != set_2.end(); j++)
		{
			vec_i = *i;
			vec_j = *j;
			vec_j.insert(vec_j.end(), vec_i.begin(), vec_i.end());
			result.push_back(vec_j);
		}
	return result;
}

vector<vector<string>> cartesian_product(vector<vector<string>> vec_1, vector<vector<string>> vec_2)
{
	vector<vector<string>> result;
	vector<string> vec_i, vec_j;
	for (auto i = vec_1.begin(); i != vec_1.end(); i++)
		for (auto j = vec_2.begin(); j != vec_2.end(); j++)
		{
			vec_i = *i;
			vec_j = *j;
			vec_j.insert(vec_j.end(), vec_i.begin(), vec_i.end());
			result.push_back(vec_j);
		}
	return result;
}

vector<vector<string>> generate_permutations_of_merge()
{
	vector<string> costs{ "+", "-", "+z", "-z" };
	int n = costs.size(), r = 2;
	vector<string> permutation(r, "");
	set<vector<string>> cost_permutations;
	permutations(r - 1, costs, permutation, cost_permutations, true);

	vector<string> statuses{ "U", "M" };
	n = statuses.size();
	r = 2;
	permutation = vector<string>(r, "");
	set<vector<string>> status_permutations;
	permutations(r - 1, statuses, permutation, status_permutations, true);

	vector<vector<string>> all_permutations;
	all_permutations = cartesian_product(status_permutations, cost_permutations);

	return all_permutations;
}

vector<vector<string>> generate_permutations_of_demerge()
{
	vector<vector<string>> before_merge_permutations = generate_permutations_of_merge();

	vector<string> costs{ "+", "-", "+z", "-z" };
	int n = costs.size(), r = 1;
	set<vector<string>> cost_permutations;
	vector<string> permutation(r, "");
	permutations(r - 1, costs, permutation, cost_permutations, true);

	vector<string> statuses{ "U", "M" };
	n = statuses.size();
	r = 1;
	set<vector<string>> status_permutations;
	permutation = vector<string>(r, "");
	permutations(r - 1, statuses, permutation, status_permutations, true);

	vector<vector<string>> after_merge_permutations = cartesian_product(status_permutations, cost_permutations);

	vector<vector<string>>
		demerge_permutations = cartesian_product(before_merge_permutations, after_merge_permutations);

	return demerge_permutations;
}

// O(3^log(k)*n*log(n) + log(k)*n^(2) + n^3)  -- n^3 is for calculating p3 for the first time when graph data is loaded
int main(int argc, char** args)
{
	string filename = "";
//	 filename = "/home/zia/studies/Algorithm_Engineering/AlgorithmEngineering/wce-students/2-real-world/w037.dimacs";
//	filename = "/home/zia/studies/Algorithm_Engineering/AlgorithmEngineering/wce-students/1-random/r043.dimacs";
//	filename = "/home/zia/studies/Algorithm_Engineering/AlgorithmEngineering/wce-students/3-actionseq/a003.dimacs";
	// ename = "/home/zia/studies/Algorithm_Engineering/AlgorithmEngineering/wce-students/2-real-world/w057.dimacs";

	processor _procs;
//	_procs.verify(filename);
	_procs.verify("");

	return 0;
}
