#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stack>

#include "cluster_graph.h"
#include <float.h>

class processor
{
	unsigned int** all_explored_statuses = NULL;

 public:
	enum explored_status : unsigned int
	{
		NONE = 0,
		ALREADY_EXPLORED_BY_DELETION = 1,
	};
	int step_count = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int c11 = 0, c22 = 0, c33 = 0, c44 = 0;
	pair<cluster_graph::merge_result, int> try_merge(int budget, cluster_graph* cg)
	{
		bool b1, b2, b3, b4;
		pair<cluster_graph::merge_result, int> result;
		for (auto i : cg->non_composed_nodes)
			for (auto j : cg->non_composed_nodes)
				if (i != j)
				{
					if (((cg->get_weight_between(i, j) > budget && cg->get_connection_connected_status_from_to(i, j))
						|| (cg->get_connection_connected_status_from_to(i, j)
							&& cg->get_connection_changed_status_from_to(i, j)) || merge_reduction_rule_2(i, j, cg)
						|| merge_reduction_rule_1(i, j, cg)
						|| all_explored_statuses[i][j] == ALREADY_EXPLORED_BY_DELETION))
					{
						result = cg->merge(i, j, budget);
						if (result.first == cluster_graph::merge_result::POSSIBLE_WITH_COST || result.first
							== cluster_graph::merge_result::NOT_POSSIBLE_EDGES_MODIFIED)        // not sure about second condition, it donot fails, but dont want to take risk
//						if (result.first == cluster_graph::merge_result::POSSIBLE_WITH_COST ||																				// it will work because otherwise solution will not not exist because it was already explored
//							(result.first == cluster_graph::merge_result::NOT_POSSIBLE_EDGES_MODIFIED																		// it is recusive
//								&& !(all_explored_statuses[i][j] == ALREADY_EXPLORED_BY_DELETION)))
							return result;
					}
				}
		return pair<cluster_graph::merge_result, int>(cluster_graph::merge_result::TOO_EXPENSIVE, -1);
	}
	p3s_bucket* p_bucket;

	bool with_merging = false;
	// O(3^k*n*log(n)), k is budget, n is no. of nodes.
	int solve(int budget, cluster_graph* cg)
	{
		step_count++;
		if (budget < 0)
			return -1;

		if (p_bucket->is_empty())
			return budget;

		if (budget < lower_bound(cg))
			return -1;

//		rec_steps++;
//		int k;
		pair<cluster_graph::merge_result, int> m_res;
		int previous_merge_nodes = cg->m;
		while (with_merging)
		{
//			k = try_merge(budget, cg);
			m_res = try_merge(budget, cg);
//			if (k != -1)
			if (m_res.first == cluster_graph::POSSIBLE_WITH_COST)
				budget -= m_res.second;
			else if (m_res.first == cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(false);
				return -1;
			}
			else if (m_res.first == cluster_graph::TOO_EXPENSIVE)
				break;
		}

//		if (step_count % 100 == 0)
		{
//			int max_connection_non_composed_node_index = -1;
//			int max_connection_count = INT32_MIN;
//			set<node_weight_pair> const* nodes;
//			for (auto i : cg->non_composed_nodes)
//			{
//				nodes = cg->get_connected_nodes_of(i);
//				if (nodes->size() > max_connection_count)
//				{
//					max_connection_count = nodes->size();
//					max_connection_non_composed_node_index = i;
//				}
//			}

			int max_cost_reduced_non_composed_node_index = -1;
			int max_cost_reduced = INT32_MIN;
			for (auto i : cg->non_composed_nodes)
			{
				m_res = cut_rule_1(i, cg, budget, true);
				if (m_res.first == cluster_graph::POSSIBLE_WITH_COST)
				{
					if (max_cost_reduced < m_res.second)
					{
						max_cost_reduced_non_composed_node_index = i;
						max_cost_reduced = m_res.second;
					}
				}
			}

			if (max_cost_reduced_non_composed_node_index != -1)
			{
				m_res = cut_rule_1(max_cost_reduced_non_composed_node_index, cg, budget);
				if (m_res.first == cluster_graph::POSSIBLE_WITH_COST)
				{
					while (cg->m != previous_merge_nodes)
						cg->demerge(true);
					return m_res.second;
				}
//				else if(m_res.first == cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED && m_res.second == -2)
//				{
//					while (cg->m != previous_merge_nodes)
//						cg->demerge(false);
//					return -1;
//				}
			}
		}


//		for (auto i : cg->non_composed_nodes)

		if (p_bucket->is_empty())
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(true);
			return budget;
		}

		if (budget < lower_bound(cg))
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(false);
			return -1;
		}

		p3 _p3 = p_bucket->retrieve_max_weight_p3();
		int u_index = _p3.u;
		int v_index = _p3.v;
		int w_index = _p3.w;

		if (!(cg->all_edge_statuses[v_index][u_index] & cluster_graph::CONNECTION_CHANGED)
			&& all_explored_statuses[v_index][u_index] != ALREADY_EXPLORED_BY_DELETION)
		{
			cg->disconnect_nodes(v_index, u_index);

			int budget_left = budget - abs(cg->get_weight_between(v_index, u_index));
			budget_left = solve(budget_left, cg);
			if (budget_left != -1)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(true);
//				rec_steps--;
				return budget_left;
			}

			cg->connect_nodes(v_index, u_index, true);
		}
		if (!(cg->all_edge_statuses[v_index][w_index] & cluster_graph::CONNECTION_CHANGED)
			&& all_explored_statuses[v_index][w_index] != ALREADY_EXPLORED_BY_DELETION)
		{
			cg->disconnect_nodes(v_index, w_index);

			int budget_left = budget - abs(cg->get_weight_between(v_index, w_index));

			all_explored_statuses[v_index][u_index] = ALREADY_EXPLORED_BY_DELETION;
			all_explored_statuses[u_index][v_index] = ALREADY_EXPLORED_BY_DELETION;
			budget_left = solve(budget_left, cg);
			all_explored_statuses[v_index][u_index] = NONE;
			all_explored_statuses[u_index][v_index] = NONE;

			if (budget_left != -1)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(true);
//				rec_steps--;
				return budget_left;
			}

			cg->connect_nodes(v_index, w_index, true);
		}
		if (!(cg->all_edge_statuses[u_index][w_index] & cluster_graph::CONNECTION_CHANGED)
			&& data_reduction_rules(u_index, w_index, cg))
		{
			cg->connect_nodes(u_index, w_index);

			int budget_left = budget - abs(cg->get_weight_between(u_index, w_index));

			all_explored_statuses[v_index][u_index] = ALREADY_EXPLORED_BY_DELETION;
			all_explored_statuses[u_index][v_index] = ALREADY_EXPLORED_BY_DELETION;
			all_explored_statuses[v_index][w_index] = ALREADY_EXPLORED_BY_DELETION;
			all_explored_statuses[w_index][v_index] = ALREADY_EXPLORED_BY_DELETION;
			budget_left = solve(budget_left, cg);
			all_explored_statuses[v_index][w_index] = NONE;
			all_explored_statuses[w_index][v_index] = NONE;
			all_explored_statuses[v_index][u_index] = NONE;
			all_explored_statuses[u_index][v_index] = NONE;

			if (budget_left != -1)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(true);
//				rec_steps--;
				return budget_left;
			}

			cg->disconnect_nodes(u_index, w_index, true);
		}
		while (cg->m != previous_merge_nodes)
			cg->demerge(false);
//		rec_steps--;
		return -1;
	}

	bool data_reduction_rules(int u, int v, cluster_graph* cg)
	{
		// return true;
		if (!cg->are_non_composed_nodes(u, v))
			int i = 1 / 0;

		if (!cg->get_connection_connected_status_from_to(u, v))
		{
			int sum = 0;
			set<node_weight_pair> const* nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes->begin(); i != nodes->end(); i++)
				if (i->node_index != v)
					sum += (*i).weight;

			if (sum <= abs(cg->get_weight_between(u, v)))
				return false;

			sum = 0;
			nodes = cg->get_connected_nodes_of(v);
			for (auto i = nodes->begin(); i != nodes->end(); i++)
				if (i->node_index != u)
					sum += (*i).weight;

			if (sum <= abs(cg->get_weight_between(u, v)))
				return false;
		}

		return true;
	}

	bool merge_reduction_rule_1(int u, int v, cluster_graph* cg)
	{
		if (!cg->are_non_composed_nodes(u, v))
			int i = 1 / 0;

		int sum = 0;
		if (cg->get_connection_connected_status_from_to(u, v))
		{
			set<node_weight_pair> const* nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes->begin(); i != nodes->end(); i++)
				if (i->node_index != v)
					sum += i->weight;
			nodes = cg->get_disconnected_nodes_of(u);
			for (auto i = nodes->begin(); i != nodes->end();
				 i++)
				if (i->node_index != v)
					sum += abs(i->weight);

			if (sum <= cg->get_weight_between(u, v))
				return true;
		}

		return false;
	}

	bool merge_reduction_rule_2(int u, int v, cluster_graph* cg)
	{
		if (!cg->are_non_composed_nodes(u, v))
			int i = 1 / 0;

		int sum = 0;
		if (cg->get_connection_connected_status_from_to(u, v))
		{
			set<node_weight_pair> const* nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes->begin(); i != nodes->end(); i++)
				if (i->node_index != v)
					sum += (*i).weight;

			nodes = cg->get_connected_nodes_of(v);
			for (auto i = nodes->begin(); i != nodes->end(); i++)
				if (i->node_index != u)
					sum += (*i).weight;

			if (sum <= cg->get_weight_between(u, v))
				return true;
		}

		return false;
	}

	pair<cluster_graph::merge_result, int> cut_rule_1(int u,
		cluster_graph* cg,
		int budget,
		bool checking_for_cost = false)
	{
		if (cg->non_composed_nodes.find(u) == cg->non_composed_nodes.end())
			int i = 1 / 0;

		set<node_weight_pair> const* connected_nodes_u = cg->get_connected_nodes_of(u);
		set<node_weight_pair>::iterator it_i, it_j;
		int cost_of_making_clique = 0;
		int total_connected_cost_u = 0;
		bool possible = true;
		for (it_i = connected_nodes_u->begin(); it_i != connected_nodes_u->end() && possible; it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
				total_connected_cost_u += cg->get_weight_between(u, it_i->node_index);
				it_j = it_i;
				it_j++;
				for (; it_j != connected_nodes_u->end() && possible; it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index)
						&& !cg->get_connection_connected_status_from_to(it_i->node_index, it_j->node_index))
					{
						if (cg->get_weight_between(it_i->node_index, it_j->node_index) > 0)
							int i = 1 / 0;
						cost_of_making_clique += abs(cg->get_weight_from_to(it_i->node_index, it_j->node_index));
						if (cg->get_connection_changed_status_from_to(it_i->node_index, it_j->node_index))
							possible = false;
					}
			}
			else int i = 1 / 0;
		int cost_of_cutting_graph = 0;
		set<node_weight_pair> const* connected_nodes_i;
		for (it_i = connected_nodes_u->begin(); it_i != connected_nodes_u->end() && possible; it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
				connected_nodes_i = cg->get_connected_nodes_of(it_i->node_index);
				for (it_j = connected_nodes_i->begin(); it_j != connected_nodes_i->end() && possible; it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index) && it_j->node_index != u
						&& !cg->get_connection_connected_status_from_to(u, it_j->node_index))
					{
						if (cg->get_weight_between(it_i->node_index, it_j->node_index) < 0)
							int i = 1 / 0;
						cost_of_cutting_graph += cg->get_weight_between(it_i->node_index, it_j->node_index);
						if (cg->get_connection_changed_status_from_to(it_i->node_index, it_j->node_index))
							possible = false;
					}
			}
			else int i = 1 / 0;
		if (!possible)
			return pair<cluster_graph::merge_result, int>(cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED, -1);

		int total_cost = cost_of_making_clique + cost_of_cutting_graph;
		if (total_cost > budget)
			return pair<cluster_graph::merge_result, int>(cluster_graph::TOO_EXPENSIVE, -1);
		if (checking_for_cost)
			return pair<cluster_graph::merge_result, int>(cluster_graph::POSSIBLE_WITH_COST, total_cost);

		if (total_cost < 0)
			int i = 1 / 0;

		set<node_weight_pair> connected_nodes_u_copy =
			cg->get_connected_nodes_copy_of(u);                // this is required so that we loop on old connection and add only those connections, otherwise we will be looping on something which is getting changed
		stack<pair<char, pair<int, int>>> changes_stack;
		for (it_i = connected_nodes_u_copy.begin(); it_i != connected_nodes_u_copy.end(); it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
				it_j = it_i;
				it_j++;
				for (; it_j != connected_nodes_u_copy.end(); it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index)
						&& !cg->get_connection_connected_status_from_to(it_i->node_index, it_j->node_index))
					{
						changes_stack
							.push(pair<char, pair<int, int>>('c', pair<int, int>(it_i->node_index, it_j->node_index)));
						cg->connect_nodes(it_i->node_index, it_j->node_index);
					}
			}
			else int i = 1 / 0;
		set<node_weight_pair> connected_nodes_i_copy;
		for (it_i = connected_nodes_u_copy.begin(); it_i != connected_nodes_u_copy.end(); it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
				connected_nodes_i_copy = cg->get_connected_nodes_copy_of(it_i->node_index);
				for (it_j = connected_nodes_i_copy.begin(); it_j != connected_nodes_i_copy.end(); it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index) && it_j->node_index != u
						&& !cg->get_connection_connected_status_from_to(u, it_j->node_index))
					{
						changes_stack
							.push(pair<char, pair<int, int>>('d', pair<int, int>(it_i->node_index, it_j->node_index)));
						cg->disconnect_nodes(it_i->node_index, it_j->node_index);
					}
			}
			else int i = 1 / 0;
		if (cg->get_connected_nodes_of(0)->size() > 0 && cg->get_connected_nodes_of(0)->begin()->node_index == 0)
			int i = 1 / 0;

		int previous_merge_nodes = cg->m;
		int last_merge_index = -1;
		pair<cluster_graph::merge_result, int> m_res;

		for (auto i : *connected_nodes_u)
			if (last_merge_index == -1)
			{
				m_res = cg->merge(u, i.node_index, 0);
				if (m_res.first != cluster_graph::POSSIBLE_WITH_COST || m_res.second != 0)
					int i = 1 / 0;
				last_merge_index = cg->n + cg->m - 1;
			}
			else
			{
				m_res = cg->merge(i.node_index, last_merge_index, 0);
				if (m_res.first != cluster_graph::POSSIBLE_WITH_COST || m_res.second != 0)
					int i = 1 / 0;
				last_merge_index++;
			}

		if (!connected_nodes_u->empty())
		{
			int budget_left = budget - total_cost;
			budget_left = solve(budget_left, cg);
			if (budget_left != -1)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(true);
				return pair<cluster_graph::merge_result, int>(cluster_graph::POSSIBLE_WITH_COST, budget_left);
			}
		}

		while (cg->m != previous_merge_nodes)
			cg->demerge(false);

		pair<char, pair<int, int>> entry;
		while (!changes_stack.empty())
		{
			entry = changes_stack.top();
			changes_stack.pop();
			if (entry.first == 'c')
				cg->disconnect_nodes(entry.second.first, entry.second.second, true);
			else if (entry.first == 'd')
				cg->connect_nodes(entry.second.first, entry.second.second, true);
		}

		return pair<cluster_graph::merge_result, int>(cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED, total_connected_cost_u > total_cost ? -2 : -1);
	}

	int lower_bound(cluster_graph* cg)
	{
		bool vertex_already_taken[2 * cg->n];

		for (int i = 0; i < 2 * cg->n; i++)
			vertex_already_taken[i] = false;

		unsigned int vertex_disjoint_lower_bound = 0;
		unsigned int min_cost;
		for (auto p3 : *p_bucket->get_all_p3s_weight_sorted())
			if (!vertex_already_taken[p3.u] && !vertex_already_taken[p3.v] && !vertex_already_taken[p3.w])
			{
				min_cost = min(cg->get_weight_from_to(p3.v, p3.u),
					min(cg->get_weight_from_to(p3.v, p3.w), -cg->get_weight_from_to(p3.u, p3.w)));
				vertex_disjoint_lower_bound += min_cost;
				vertex_already_taken[p3.u] = true;
				vertex_already_taken[p3.v] = true;
				vertex_already_taken[p3.w] = true;
			}

		bool edge_already_taken[2 * cg->n][2 * cg->n];

		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				edge_already_taken[i][j] = false;

		set<p3> edge_disjoint_p3s;
		for (auto p3 : *p_bucket->get_all_p3s_weight_sorted())
			if (!edge_already_taken[p3.v][p3.u] && !edge_already_taken[p3.v][p3.w] && !edge_already_taken[p3.u][p3.w])
			{
				edge_disjoint_p3s.insert(p3);
				edge_already_taken[p3.v][p3.u] = true;
				edge_already_taken[p3.v][p3.w] = true;
				edge_already_taken[p3.u][p3.w] = true;
			}

		unsigned int edge_p3s_count[2 * cg->n][2 * cg->n];

		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				edge_p3s_count[i][j] = 0;

		for (auto p3 : edge_disjoint_p3s)
		{
			edge_p3s_count[p3.v][p3.u]++;
			edge_p3s_count[p3.v][p3.w]++;
			edge_p3s_count[p3.u][p3.w]++;
		}

		float relative_edge_cost[2 * cg->n][2 * cg->n];
		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				if (edge_already_taken[i][j])
					relative_edge_cost[i][j] = (abs(cg->get_weight_between(i, j)) / edge_p3s_count[i][j]);

		float min_relative_cost = FLT_MAX;
		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				if (edge_already_taken[i][j])
					min_relative_cost = min(min_relative_cost, relative_edge_cost[i][j]);

		int p3s_count = edge_disjoint_p3s.size();
		float edge_disjoint_lower_bound = min_relative_cost != FLT_MAX ? (p3s_count * min_relative_cost) : 0;

//		return vertex_disjoint_lower_bound > edge_disjoint_lower_bound ? vertex_disjoint_lower_bound :
//			edge_disjoint_lower_bound;
		return vertex_disjoint_lower_bound;
	}

	int binary_search_for_optimal_k(int l, int r, cluster_graph* cg)
	{ // r is always considered to be the solution, just not always the optimal
		if (l == r)
			return r;

		int m = (l + r) / 2;
		int ret = solve(m, cg);
		if (ret == -1)
			return binary_search_for_optimal_k(m + 1, r, cg);
		else
		{
			cg->reset_graph(); // resetting graph to starting position after finding solution
			return binary_search_for_optimal_k(l, m, cg);
		}
	}

	void verify(string file_name, bool with_mergin = true) // file_name is "" if asked to read from cin
	{
		cluster_graph* cg;
		cg = new cluster_graph();
		cg->load_graph(file_name);
		p_bucket = cg->get_p3_bucket();
		with_merging = true;

		if (all_explored_statuses == NULL)
		{
			all_explored_statuses = new unsigned int* [2 * cg->n];
			for (int i = 0; i < 2 * cg->n; i++)
				all_explored_statuses[i] = new unsigned int[2 * cg->n];
		}

		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				all_explored_statuses[i][j] = 0;

		int k;
		for (k = 1; solve(k, cg) == -1; k *= 2); // if no solution is found, graph remains in original state
		cg->reset_graph();

		k = binary_search_for_optimal_k(k / 2 + 1, k, cg);
//		step_count = 0;
		solve(k, cg);
		// cout <<    "-------------------------------------------------\n";
		int changed_edges_cost = 0;
		for (int i = 0; i < cg->n; i++)
		{
			for (int j = i + 1; j < cg->n; j++)
				if (cg->all_edge_statuses[i][j] & 0b100)
				{
					cout << i + 1 << " " << j + 1 << endl;
					if ((cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED)
						&& cg->get_connected_nodes_of(i)->find(node_weight_pair(j, 0))
							== cg->get_connected_nodes_of(i)->end())
						int i = 1 / 0;
					if (!(cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED)
						&& cg->get_disconnected_nodes_of(i)->find(node_weight_pair(j, 0))
							== cg->get_disconnected_nodes_of(i)->end())
						int i = 1 / 0;
					changed_edges_cost += (cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED) ?
						abs(cg->get_connected_nodes_of(i)->find(node_weight_pair(j, 0))->weight) :
						abs(cg->get_disconnected_nodes_of(i)->find(node_weight_pair(j, 0))->weight);
				}
		}
		cout << "#recursive steps: " << step_count << endl;
		// cout << "#recursive steps: " << cg->c11 << "/" << cg->c1 << endl;
		// cout << "#recursive steps: " << cg->c22 << "/" << cg->c2 << endl;
		// cout << "#recursive steps: " << cg->c33 << "/" << cg->c3 << endl;
		// cout << "#recursive steps: " << cg->c44 << "/" << cg->c4 << endl;

		// cout << "-------------------------------------------------\n";

		int p3_count = 0;
		for (int i = 0; i < cg->n; i++)
		{
			for (int j = 0; j < cg->n; j++)
				if (j != i && cg->all_edge_statuses[i][j] & 0b010)
				{
					for (int k = j + 1; k < cg->n; k++)
						if (k != i && cg->all_edge_statuses[i][k] & 0b010)
						{
							if (!(cg->all_edge_statuses[j][k] & 0b010))
								p3_count++;
						}
				}
		}

		delete cg;

		if (file_name != "")
		{
			if (p3_count == 0 && changed_edges_cost == k)
				cout << "verfied at cost: " << changed_edges_cost << endl;
			else
				cout << "verification failed at cost: " << k << ", with edge modified cost: " << changed_edges_cost
					 << ", p3 count: " << p3_count << endl;
		}
	}
};

#endif