#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stack>
#include <map>
#include <cmath>
#include <float.h>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/typeof/typeof.hpp>

#include "cluster_graph.h"
#include "cplex.h"

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

	int depth = -1;
	int max_depth = -1;
	bool with_merging = false;
	// O(3^k*n*log(n)), k is budget, n is no. of nodes.
	int solve(int budget, cluster_graph* cg)
	{
		depth++;
		step_count++;
		if (budget < 0)
		{
			depth--;
			return -1;
		}

		if (p_bucket->is_empty())
		{
			depth--;
			return budget;
		}

		if (solver_optimal && budget < lower_bound(cg))
		{
			depth--;
			return -1;
		}

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
				depth--;
				return -1;
			}
			else if (m_res.first == cluster_graph::TOO_EXPENSIVE)
				break;
		}

		if (p_bucket->is_empty())
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(true);
			depth--;
			return budget;
		}


		max_depth = max_depth < depth ? depth : max_depth;
		int c = cg->n * 0.5;
//		if(depth % (c == 0 ? 1 : c) == 0 && false)
//		if (step_count % 200 == 0)
//		if (cg->non_composed_nodes.size() < cg->n/2)
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
//					cout << "aafdsafdsa" << endl;
					if (max_cost_reduced < cg->get_connected_nodes_of(i)->size())
					{
						max_cost_reduced_non_composed_node_index = i;
						max_cost_reduced = cg->get_connected_nodes_of(i)->size();
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
					depth--;
					return m_res.second;
				}
				else if(m_res.first == cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED && m_res.second == -2)
				{
					while (cg->m != previous_merge_nodes)
						cg->demerge(false);
					depth--;
					return -1;
				}
			}
		}

		if (p_bucket->is_empty())
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(true);
			depth--;
			return budget;
		}

		if (solver_optimal && budget < lower_bound(cg))
		{
			while (cg->m != previous_merge_nodes)
				cg->demerge(false);
			depth--;
			return -1;
		}

//		if(use_cplex && depth > 0.4*cg->n)
			if(solver_with_cplex && depth > 0.5*cg->n)
		{
			pair<int, set<pair<int, int>>> solution = cplex_solve(cg);
			if (solution.first != -1 && solution.first <= budget)
			{
				for (auto e : solution.second)
					cg->flip_connection_between(e.first, e.second);

				while (cg->m != previous_merge_nodes)
					cg->demerge(true);

				depth--;
				return budget - solution.first;
			}
			else
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(false);
				depth--;
				return -1;
			}
		}

		p3 _p3 = p_bucket->retrieve_max_weight_p3();
		int u_index = _p3.u;
		int v_index = _p3.v;
		int w_index = _p3.w;

		int u = _p3.u;
		int v = _p3.v;
		int w = _p3.w;

		if (cg->get_weight_between(v, u) < cg->get_weight_between(v, w))
		{
			u_index = w;
			w_index = u;
		}

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
				depth--;
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
				depth--;
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
				depth--;
				return budget_left;
			}

			cg->disconnect_nodes(u_index, w_index, true);
		}
		while (cg->m != previous_merge_nodes)
			cg->demerge(false);
//		rec_steps--;
		depth--;
		return -1;
	}

	int solve_branching_2(cluster_graph *cg)
	{
		pair<int, list<pair<char, pair<int, int>>>> solution = solve_branching_2_helper(cg, 0, cplex_solve(cg).first);

		for(auto it = solution.second.begin(); it != solution.second.end(); it++)
			if(it->first == 'm')
				cg->merge(it->second.first, it->second.second, INT32_MAX);
			else if(it->first == 'd')
				cg->disconnect_nodes(it->second.first, it->second.second);

		while(cg->m != 0)
			cg->demerge(true);

		return solution.first;
	}

	pair<int, list<pair<char, pair<int, int>>>> solve_branching_2_helper(cluster_graph *cg, int current_cost, int upper_bound)
	{
		if(lower_bound(cg) + current_cost > upper_bound)
			return pair<int, list<pair<char, pair<int, int>>>>(-1, list<pair<char, pair<int, int>>>());
		if(p_bucket->is_empty())
			return pair<int, list<pair<char, pair<int, int>>>>(current_cost, list<pair<char, pair<int, int>>>());

		int previous_merge_nodes = cg->m;
		pair<cluster_graph::merge_result, int> m_res;
		list<pair<char, pair<int, int>>> reduction_steps = list<pair<char, pair<int, int>>>();
		while (true)
		{
			m_res = try_merge(INT_MAX, cg);
			if (m_res.first == cluster_graph::POSSIBLE_WITH_COST)
			{
				current_cost += m_res.second;
			}
			else if (m_res.first == cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED)
			{
				while (cg->m != previous_merge_nodes)
					cg->demerge(false);
				return pair<int, list<pair<char, pair<int, int>>>>(-1, list<pair<char, pair<int, int>>>());
			}
			else if (m_res.first == cluster_graph::TOO_EXPENSIVE)
				break;
		}

		if(lower_bound(cg) + current_cost > upper_bound)
		{
			while (cg->m != previous_merge_nodes) cg->demerge(false);
			return pair<int, list < pair<char, pair<int, int>>>>
			(-1, list<pair<char, pair<int, int>>>());
		}

		pair<int, list < pair<char, pair<int, int>>>> final_solution = pair<int, list<pair<char, pair<int, int>>>>(current_cost, list<pair<char, pair<int, int>>>());

		if(!p_bucket->is_empty())
		{
			p3 _p3 = p_bucket->retrieve_max_weight_p3();
			int u = _p3.u;
			int v = _p3.v;
			int w = _p3.w;

			if(cg->get_weight_between(v, u) < cg->get_weight_from_to(v, w))
				u = w;

			pair<int, list < pair<char, pair<int, int>>>>
			solution_1, solution_2;
			solution_1 = pair<int, list < pair<char, pair<int, int>>>>
			(-1, list<pair<char, pair<int, int>>>());
			solution_2 = pair<int, list < pair<char, pair<int, int>>>>
			(-1, list<pair<char, pair<int, int>>>());

			m_res = cg->merge(u, v, INT32_MAX);
			if (m_res.first == cluster_graph::POSSIBLE_WITH_COST)
			{
//			int merge_index = cg->n + cg->m - 1;
				solution_1 = solve_branching_2_helper(cg, current_cost + m_res.second, upper_bound);
				cg->demerge(false);
			}
			if (!cg->get_connection_changed_status_from_to(u, v))
			{
				cg->disconnect_nodes(u, v);
				solution_2 =
					solve_branching_2_helper(cg, current_cost + abs(cg->get_weight_from_to(u, v)), upper_bound);
				cg->connect_nodes(u, v, true);
			}

			final_solution = pair<int, list < pair<char, pair<int, int>>>>
			(-1, list<pair<char, pair<int, int>>>());
			char solution_type = ' ';

			if (solution_1.first != -1 && solution_2.first != -1)
			{
				final_solution = solution_1.first < solution_2.first ? solution_1 : solution_2;
				solution_type = solution_1.first < solution_2.first ? 'm' : 'd';
			}
			else if (solution_1.first == -1)
			{
				final_solution = solution_2;
				solution_type = 'd';
			}
			else if (solution_2.first == -1)
			{
				final_solution = solution_1;
				solution_type = 'm';
			}

			if (solution_type != ' ')
				final_solution.second.push_front(pair<char, pair<int, int>>(solution_type, pair<int, int>(u, v)));
		}

		while(cg->m != previous_merge_nodes)
		{
			node* merged_node = cg->get_node_by_index(cg->n + cg->m - 1);
			if (final_solution.first != -1)
				final_solution.second.push_front(pair<char, pair<int, int>>('m', pair<int, int>(merged_node->composed_node_index_1, merged_node->composed_node_index_2)));
			cg->demerge(false);
		}

		return final_solution;
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

	struct edge_t
	{
		unsigned long first;
		unsigned long second;
	};

	int minimum_cut_cost(int u, cluster_graph *cg)
	{
		typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
									  boost::no_property, boost::property<boost::edge_weight_t, int> > undirected_graph;
		typedef boost::property_map<undirected_graph, boost::edge_weight_t>::type weight_map_type;
		typedef boost::property_traits<weight_map_type>::value_type weight_type;

		// define the 16 edges of the graph. {3, 4} means an undirected edge between vertices 3 and 4.
//		edge_t edges[] = {{3, 4}, {3, 6}, {3, 5}, {0, 4}, {0, 1}, {0, 6}, {0, 7},
//			{0, 5}, {0, 2}, {4, 1}, {1, 6}, {1, 5}, {6, 7}, {7, 5}, {5, 2}, {3, 4}};
		set<node_weight_pair> const* u_connected = cg->get_connected_nodes_of(u);
		int n = u_connected->size() + 1;
		if (n < 2) return -1;
		int total_edges = ((n - 1) * n) / 2;
		edge_t edges[total_edges];

		map<int, int> graph_to_boost_node_index;
		map<int, int> boost_to_graph_node_index;

		int node_count = 0;
		graph_to_boost_node_index[u] = node_count;
		boost_to_graph_node_index[node_count] = u;
		node_count++;
		for (auto i : *u_connected)
		{
			graph_to_boost_node_index[i.node_index] = node_count;
			boost_to_graph_node_index[node_count] = i.node_index;
			node_count++;
		}

		if (node_count != n)
			int i = 1 / 0;

		// for each of the 16 edges, define the associated edge weight. ws[i] is the weight for the edge
		// that is described by edges[i].
//		weight_type ws[] = {0, 3, 1, 3, 1, 2, 6, 1, 8, 1, 1, 80, 2, 1, 1, 4};
		weight_type ws[total_edges];

		set<node_weight_pair>::iterator it_i, it_j;
		unsigned int idx_1, idx_2;
		int edge_count = 0;
		for (it_i = u_connected->begin(); it_i != u_connected->end(); it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
				idx_1 = graph_to_boost_node_index.find(u)->second;
				idx_2 = graph_to_boost_node_index.find(it_i->node_index)->second;
				edges[edge_count].first = idx_1;
				edges[edge_count].second = idx_2;
				ws[edge_count] = cg->get_connection_connected_status_from_to(u, it_i->node_index) ?
					cg->get_weight_between(u, it_i->node_index) : 0;
				edge_count++;

				it_j = it_i;
				it_j++;
				for (; it_j != u_connected->end(); it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index))
					{
						idx_1 = graph_to_boost_node_index.find(it_i->node_index)->second;
						idx_2 = graph_to_boost_node_index.find(it_j->node_index)->second;
						edges[edge_count].first = idx_1;
						edges[edge_count].second = idx_2;
						ws[edge_count] =
							cg->get_connection_connected_status_from_to(it_i->node_index, it_j->node_index) ?
								cg->get_weight_between(it_i->node_index, it_j->node_index) : 0;
						edge_count++;
					}
					else
						int i = 1 / 0;
			}
			else int i = 1 / 0;

		if (edge_count != total_edges)
			int i = 1 / 0;

		// construct the graph object. 8 is the number of vertices, which are numbered from 0
		// through 7, and 16 is the number of edges.
//		undirected_graph g(edges, edges + 16, ws, 8, 16);
		undirected_graph g(edges, edges + total_edges, ws, n, total_edges);

//		cout << num_vertices(g) << endl;
		// define a property map, `parities`, that will store a boolean value for each vertex.
		// Vertices that have the same parity after `stoer_wagner_min_cut` runs are on the same side of the min-cut.
		BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(g), get(boost::vertex_index, g)));

		// run the Stoer-Wagner algorithm to obtain the min-cut weight. `parities` is also filled in.
		int w = boost::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::parity_map(parities));

//		return w;

//		cout << "The min-cut weight of G is " << w << ".\n" << endl;
//		assert(w == 7);

//		cout << "One set of vertices consists of:" << endl;
//		size_t i;
//		for (i = 0; i < num_vertices(g); ++i) {
//			if (get(parities, i))
//				cout << i << endl;
//		}
//		cout << endl;
//
//		cout << "The other set of vertices consists of:" << endl;
//		for (i = 0; i < num_vertices(g); ++i) {
//			if (!get(parities, i))
//				cout << i << endl;
//		}
//		cout << endl;

		set<int> vertex_set_1, vertex_set_2;;
		for (int i = 0; i < num_vertices(g); ++i)
			if (get(parities, i))
				vertex_set_1.insert(boost_to_graph_node_index.find(i)->second);
			else
				vertex_set_2.insert(boost_to_graph_node_index.find(i)->second);

		for(auto i : vertex_set_1)
			for(auto j : vertex_set_2)
			{
				if(i != u && cg->get_connected_nodes_of(u)->find(node_weight_pair(i, 0)) == cg->get_connected_nodes_of(u)->end())
					int k=1/0;
				if(j != u && cg->get_connected_nodes_of(u)->find(node_weight_pair(j, 0)) == cg->get_connected_nodes_of(u)->end())
					int k=1/0;
				if(!(!cg->get_connection_changed_status_from_to(i, j) || !cg->get_connection_connected_status_from_to(i, j)))
					return -1;
				}
		return w;
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
//				if (cg->get_weight_between(u, it_i->node_index) == 0)
//					possible = false;
				total_connected_cost_u += cg->get_weight_between(u, it_i->node_index);
				it_j = it_i;
				it_j++;
				for (; it_j != connected_nodes_u->end() && possible; it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index))
					{
						if (!cg->get_connection_connected_status_from_to(it_i->node_index, it_j->node_index))
						{
							if (cg->get_weight_between(it_i->node_index, it_j->node_index) > 0)
								int i = 1 / 0;
							cost_of_making_clique += abs(cg->get_weight_from_to(it_i->node_index, it_j->node_index));
							if (cg->get_connection_changed_status_from_to(it_i->node_index, it_j->node_index))
								possible = false;
						}
//						if (cg->get_weight_between(it_i->node_index, it_j->node_index) == 0)
//							possible = false;
					}
					else int i = 1 / 0;
			}
			else int i = 1 / 0;
		int cost_of_cutting_graph = 0;
		set<node_weight_pair> const* connected_nodes_i;
		for (it_i = connected_nodes_u->begin(); it_i != connected_nodes_u->end() && possible; it_i++)
			if (cg->are_non_composed_nodes(u, it_i->node_index))
			{
//				if (cg->get_weight_between(u, it_i->node_index) == 0)
//					possible = false;
				connected_nodes_i = cg->get_connected_nodes_of(it_i->node_index);
				for (it_j = connected_nodes_i->begin(); it_j != connected_nodes_i->end() && possible; it_j++)
					if (cg->are_non_composed_nodes(it_i->node_index, it_j->node_index) && it_j->node_index != u
						&& !cg->get_connection_connected_status_from_to(u, it_j->node_index))
					{
						if (cg->get_weight_between(it_i->node_index, it_j->node_index) < 0)
							int i = 1 / 0;
						cost_of_cutting_graph += cg->get_weight_between(it_i->node_index, it_j->node_index);
					}
			}
			else int i = 1 / 0;

//		possible &= ((2 * cost_of_making_clique + cost_of_cutting_graph) < total_connected_cost_u);
		possible &= (cost_of_making_clique + cost_of_cutting_graph < minimum_cut_cost(u, cg));

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

		return pair<cluster_graph::merge_result, int>(cluster_graph::NOT_POSSIBLE_EDGES_MODIFIED,
			-2);
	}

	unsigned int lower_bound(cluster_graph* cg)
	{
		const set<p3> *p3s = p_bucket->get_all_p3s_weight_sorted();
//		set<p3> non_zero_edge_p3s;
//		set<p3> zero_edge_p3s;
//		for(auto p3 : *p3s)
//			if(cg->get_weight_between(p3.v, p3.u) == 0 || cg->get_weight_between(p3.v, p3.w) == 0 || cg->get_weight_between(p3.u, p3.w) == 0)
//				zero_edge_p3s.insert(p3);
//			else
//				non_zero_edge_p3s.insert(p3);
//
//		unsigned int lb_1_non_zero = lower_bound_1(cg, &non_zero_edge_p3s);
//		unsigned int lb_1_zero = lower_bound_2(cg, &zero_edge_p3s);
//		unsigned int lb_1 = (lb_1_non_zero > lb_1_zero ? lb_1_non_zero : lb_1_zero);


		unsigned int lb_1 = lower_bound_1(cg, p3s);
		unsigned int lb_2 = lower_bound_2(cg, p3s);
		unsigned int lb_3 = cplex_lower_bound(cg).first;

		return max(lb_1, max(lb_2, lb_3));
	}

	unsigned int lower_bound_1(cluster_graph* cg, const set<p3>* p3s)
	{
		unsigned int edge_p3s_count[2 * cg->n][2 * cg->n];
		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				edge_p3s_count[i][j] = 0;

		for (auto p3 : *p3s)
		{
			edge_p3s_count[p3.v][p3.u]++;
			edge_p3s_count[p3.u][p3.v]++;
			edge_p3s_count[p3.v][p3.w]++;
			edge_p3s_count[p3.w][p3.v]++;
			edge_p3s_count[p3.u][p3.w]++;
			edge_p3s_count[p3.w][p3.u]++;
		}

		float relative_edge_cost[2 * cg->n][2 * cg->n];
		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				if (edge_p3s_count[i][j])
					relative_edge_cost[i][j] = (abs(cg->get_weight_between(i, j)) / edge_p3s_count[i][j]);

		float min_relative_cost = FLT_MAX;
		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				if (edge_p3s_count[i][j])
					min_relative_cost = min(min_relative_cost, relative_edge_cost[i][j]);

		int p3s_count = p3s->size();
		unsigned int lower_bound_1 = (min_relative_cost != FLT_MAX ? ceil(p3s_count * min_relative_cost) : 0);
	}

	unsigned int lower_bound_2(cluster_graph* cg, const set<p3>* p3s)
	{
		bool edge_already_taken[2 * cg->n][2 * cg->n];

		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				edge_already_taken[i][j] = false;

		set<p3> edge_disjoint_p3s;
		for (auto p3 : *p3s)
			if (!edge_already_taken[p3.v][p3.u] && !edge_already_taken[p3.v][p3.w] && !edge_already_taken[p3.u][p3.w])
			{
				edge_disjoint_p3s.insert(p3);
				edge_already_taken[p3.v][p3.u] = true;
				edge_already_taken[p3.u][p3.v] = true;
				edge_already_taken[p3.v][p3.w] = true;
				edge_already_taken[p3.w][p3.v] = true;
				edge_already_taken[p3.u][p3.w] = true;
				edge_already_taken[p3.w][p3.u] = true;
			}

		unsigned int edge_disjoint_lower_bound_2 = 0;
		for (auto p3 : edge_disjoint_p3s)
		{
			int min_c = min(cg->get_weight_from_to(p3.v, p3.u),
				min(cg->get_weight_from_to(p3.v, p3.w), -cg->get_weight_from_to(p3.u, p3.w)));
			if (min_c < 0)
				int i = 1 / 0;
			edge_disjoint_lower_bound_2 += min_c;
		}

		return edge_disjoint_lower_bound_2;
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

	bool use_cplex_without_reduction_rules = false;
	bool use_cplex_with_reduction_rules = false;
	bool solver_optimal = false;
	bool solver_non_optimal = false;
	bool solver_with_cplex = false;
	bool solver_branching_2 = false;

	void verify(string file_name, cluster_graph *cg, mutex *_mutex, bool with_mergin = true) // file_name is "" if asked to read from cin
	{
		_mutex->lock();
		cg->load_graph(file_name);
		_mutex->unlock();

		if (all_explored_statuses == NULL)
		{
			all_explored_statuses = new unsigned int* [2 * cg->n];
			for (int i = 0; i < 2 * cg->n; i++)
				all_explored_statuses[i] = new unsigned int[2 * cg->n];
		}

		for (int i = 0; i < 2 * cg->n; i++)
			for (int j = 0; j < 2 * cg->n; j++)
				all_explored_statuses[i][j] = 0;


		p_bucket = cg->get_p3_bucket();
		with_merging = true;

		if(SOLVER_OPTIMAL)
			solver_optimal = true;
		else if(SOLVER_NON_OPTIMAL)
			solver_non_optimal = true;
		else if(SOLVER_OPTIMAL_WITH_CPLEX)
		{
			solver_optimal = true;
			solver_with_cplex = true;
		}
		else if(CPLEX_WITH_REDUCTION_RULES)
			use_cplex_with_reduction_rules = true;
		else if(CPLEX_WITHOUT_REDUCTION_RULES)
			use_cplex_without_reduction_rules = true;
		else if(SOLVER_BRANCHING_2)
			solver_branching_2 = true;

		int k;
		if(use_cplex_without_reduction_rules || use_cplex_with_reduction_rules)
		{
			if(use_cplex_without_reduction_rules)
			{
				pair<int, set<pair<int, int>>> solution = cplex_solve(cg);
				k = solution.first;
				for (auto e : solution.second)
					cg->flip_connection_between(e.first, e.second);
			}
			else
			{
				pair<cluster_graph::merge_result, int> m_res;
				int previous_merge_nodes = cg->m;
				bool merge_successfull;
				while (with_merging)
				{
					pair<cluster_graph::merge_result, int> result;
					merge_successfull = false;
					int i, j;
					set<int>::iterator it_i, it_j;
					for (it_i = cg->non_composed_nodes.begin(); it_i != cg->non_composed_nodes.end() && !merge_successfull; it_i++)
						for (it_j = cg->non_composed_nodes.begin(); it_j != cg->non_composed_nodes.end() && !merge_successfull; it_j++)
						{
							i = *it_i;
							j = *it_j;
							if (i != j)
							{
								if (merge_reduction_rule_1(i, j, cg) && merge_reduction_rule_2(i, j, cg))
								{
									result = cg->merge(i, j, INT_MAX);
									if (result.first
										!= cluster_graph::merge_result::POSSIBLE_WITH_COST)        // not sure about second condition, it donot fails, but dont want to take risk
										int i = 1 / 0;
									merge_successfull = true;
								}
							}
						}

					if(!merge_successfull)
						break;
				}

				set<pair<int, int>> reduction_rule_1_violation;
				for(auto i : cg->non_composed_nodes)
					for(auto j : cg->non_composed_nodes)
						if(i != j && !cg->get_connection_connected_status_from_to(i, j) && !data_reduction_rules(i, j, cg))
							reduction_rule_1_violation.insert(pair<int, int>(i, j));


				if (p_bucket->is_empty())
				{
					while (cg->m != previous_merge_nodes)
						cg->demerge(true);
				}
				else
				{
					for(auto p : reduction_rule_1_violation)
						cg->all_edge_statuses[p.first][p.second] |= cg->CONNECTION_CHANGED;
					pair<int, set<pair<int, int>>> solution = cplex_solve(cg);
					for(auto p : reduction_rule_1_violation)
						cg->all_edge_statuses[p.first][p.second] &= (cg->CONNECTION_PRESENT | cg->CONNECTION_CONNECTED | 0);

					k = solution.first;
					for (auto e : solution.second)
						cg->flip_connection_between(e.first, e.second);

					while (cg->m != previous_merge_nodes)
						cg->demerge(true);
				}
			}
		}
		else if(solver_optimal || solver_non_optimal)
		{
			k = INT32_MAX;
			if(solver_optimal == solver_non_optimal)
				int i=1/0;
			if (solver_optimal)
			{
				for (k = 1; solve(k, cg) == -1; k *= 2); // if no solution is found, graph remains in original state
				cg->reset_graph();

				k = binary_search_for_optimal_k(k / 2 + 1, k, cg);
				step_count = 0;
			}
			solve(k, cg);
		}
		else if(solver_branching_2)
		{
			k = solve_branching_2(cg);
		}
		// cout <<    "-------------------------------------------------\n";

		_mutex->lock();

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

		exit(0);
		_mutex->unlock();
	}
};

#endif