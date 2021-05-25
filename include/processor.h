#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "cluster_graph.h"

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
					if (result.first == cluster_graph::merge_result::POSSIBLE_WITH_COST || result.first == cluster_graph::merge_result::NOT_POSSIBLE_EDGES_MODIFIED)		// not sure about second condition, it donot fails, but dont want to take risk
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

		if (p_bucket->is_empty())
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(true);
//			rec_steps--;
			return budget;
		}

		p3 _p3 = p_bucket->retrieve_max_weight_p3();
		int u_index = _p3.u;
		int v_index = _p3.v;
		int w_index = _p3.w;

		if (!(cg->all_edge_statuses[v_index][u_index] & cluster_graph::CONNECTION_CHANGED)
			&& all_explored_statuses[v_index][u_index] != ALREADY_EXPLORED_BY_DELETION)
		{
			cg->disconnect_nodes(v_index,
				u_index,
				cluster_graph::CONNECTION_PRESENT | 0 | cluster_graph::CONNECTION_CHANGED);

			int budget_left = budget - abs(cg->get_weight_between(v_index, u_index));
			budget_left = solve(budget_left, cg);
			if (budget_left != -1)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(true);
//				rec_steps--;
				return budget_left;
			}

			cg->connect_nodes(v_index,
				u_index,
				cluster_graph::CONNECTION_PRESENT | cluster_graph::CONNECTION_CONNECTED | 0);
		}
		if (!(cg->all_edge_statuses[v_index][w_index] & cluster_graph::CONNECTION_CHANGED)
			&& all_explored_statuses[v_index][w_index] != ALREADY_EXPLORED_BY_DELETION)
		{
			cg->disconnect_nodes(v_index,
				w_index,
				cluster_graph::CONNECTION_PRESENT | 0 | cluster_graph::CONNECTION_CHANGED);

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

			cg->connect_nodes(v_index,
				w_index,
				cluster_graph::CONNECTION_PRESENT | cluster_graph::CONNECTION_CONNECTED | 0);
		}
		if (!(cg->all_edge_statuses[u_index][w_index] & cluster_graph::CONNECTION_CHANGED)
			&& data_reduction_rules(u_index, w_index, cg))
		{
			cg->connect_nodes(u_index,
				w_index,
				cluster_graph::CONNECTION_PRESENT | cluster_graph::CONNECTION_CONNECTED
					| cluster_graph::CONNECTION_CHANGED);

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

			cg->disconnect_nodes(u_index, w_index, cluster_graph::CONNECTION_PRESENT | 0 | 0);
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
			set<node_weight_pair> nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes.begin(); i != nodes.end(); i++)
				if (i->node_index != v)
					sum += (*i).weight;

			if (sum <= abs(cg->get_weight_between(u, v)))
				return false;

			sum = 0;
			nodes = cg->get_connected_nodes_of(v);
			for (auto i = nodes.begin(); i != nodes.end(); i++)
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
			set<node_weight_pair> nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes.begin(); i != nodes.end(); i++)
				if (i->node_index != v)
					sum += i->weight;
			nodes = cg->get_disconnected_nodes_of(u);
			for (auto i = nodes.begin(); i != nodes.end();
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
			set<node_weight_pair> nodes = cg->get_connected_nodes_of(u);
			for (auto i = nodes.begin(); i != nodes.end(); i++)
				if (i->node_index != v)
					sum += (*i).weight;

			nodes = cg->get_connected_nodes_of(v);
			for (auto i = nodes.begin(); i != nodes.end(); i++)
				if (i->node_index != u)
					sum += (*i).weight;

			if (sum <= cg->get_weight_between(u, v))
				return true;
		}

		return false;
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
		step_count = 0;
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
						&& cg->get_connected_nodes_of(i).find(node_weight_pair(j, 0))
							== cg->get_connected_nodes_of(i).end())
						int i = 1 / 0;
					if (!(cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED)
						&& cg->get_disconnected_nodes_of(i).find(node_weight_pair(j, 0))
							== cg->get_disconnected_nodes_of(i).end())
						int i = 1 / 0;
					changed_edges_cost += (cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED) ?
						abs(cg->get_connected_nodes_of(i).find(node_weight_pair(j, 0))->weight) :
						abs(cg->get_disconnected_nodes_of(i).find(node_weight_pair(j, 0))->weight);
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