#ifndef CPLEX_H
#define CPLEX_H

#include <ilcplex/ilocplex.h>
#include <map>
#include "cluster_graph.h"

ILOSTLBEGIN

pair<int, set<pair<int, int>>> cplex_solve(cluster_graph* cg)
{
	int n = cg->non_composed_nodes.size();
	map<int, int> from_graph_to_cplex_node_index;
	map<int, int> from_cplex_to_graph_node_index;

	set<int> const* nodes = &cg->non_composed_nodes;
	int i = 0;
	for (auto ni : *nodes)
	{
		from_graph_to_cplex_node_index.insert(pair<int, int>(ni, i));
		from_cplex_to_graph_node_index.insert(pair<int, int>(i, ni));
		i++;
	}

	IloEnv env;
	try
	{
		IloModel model(env);
		IloBoolVarArray vars(env, n*n);

//		for (int i = 0; i < n; i++)
//			for (int j = 0; j < n; j++)
//				vars.add(IloNumVar(env, 0.0, 1.0));

		IloNumExpr iloNumExpr(env);
		unsigned long long _uv;
		unsigned cg_u, cg_v;
		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
				cg_u = from_cplex_to_graph_node_index.find(u)->second;
				cg_v = from_cplex_to_graph_node_index.find(v)->second;
				if (cg->get_connection_connected_status_from_to(cg_u, cg_v))
					iloNumExpr += (cg->get_weight_between(cg_u, cg_v) * (1 - vars[_uv]));
				else
					iloNumExpr += (-cg->get_weight_between(cg_u, cg_v) * (vars[_uv]));

//				if(cg->get_connection_changed_status_from_to(cg_u, cg_v))
//					int i=1/0;

				if (cg->get_connection_changed_status_from_to(cg_u, cg_v))
					model.add(vars[_uv] == cg->get_connection_connected_status_from_to(cg_u, cg_v));
			}
		model.add(IloMinimize(env, iloNumExpr));

		unsigned long long vu, vw, uw;
		for (int u = 0; u < n; u++)
			for (int v = 0; v < n; v++)
				for (int w = 0; w < n; w++)
				{
					vu = v * n + u;
					vw = v * n + w;
					uw = u * n + w;
					model.add(vars[vu] + vars[vw] - vars[uw] <= 1);
					model.add(vars[vu] - vars[vw] + vars[uw] <= 1);
					model.add(-vars[vu] + vars[vw] + vars[uw] <= 1);
//					model.add(IloAbs(vars[vu] - 0.5) == 0.5);
//					model.add(IloAbs(vars[vw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[uw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[vw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[uw] - 0.5) == 0.5);
				}

		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::Param::Simplex::Display, 0);
//		cplex.setParam(IloCplex::Param::Simplex::Tolerances::Markowitz, 0.0001);
		if (!cplex.solve())
		{
			env.error() << "Failed to optimize LP." << endl;
			env.end();
			return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
			throw (-1);
		}

		IloNumArray vals(env);
		cplex.getValues(vals, vars);

		set<pair<int, int>> solution;
		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
				cg_u = from_cplex_to_graph_node_index.find(u)->second;
				cg_v = from_cplex_to_graph_node_index.find(v)->second;
//				if(vals[_uv] != 0 && vals[_uv] != 1)
//					int i= 1/0;
				if (cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) < abs(vals[_uv] - 1))
					solution.insert(pair<int, int>(cg_u, cg_v));
				if (!cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) > abs(vals[_uv] - 1) )
					solution.insert(pair<int, int>(cg_u, cg_v));
			}

		int v = cplex.getObjValue();
		env.end();
		return pair<int, set<pair<int, int>>>(v, solution);
	}
	catch (IloException& e)
	{
		env.end();
		return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...)
	{
		env.end();
		return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
		cerr << "Unknown exception caught" << endl;
	}

	env.end();

	return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
}


pair<int, set<pair<int, int>>> cplex_lower_bound(cluster_graph* cg)
{
	int n = cg->non_composed_nodes.size();
	map<unsigned int, unsigned int> from_graph_to_cplex_node_index;
	map<unsigned int, unsigned int> from_cplex_to_graph_node_index;

	set<int> const* nodes = &cg->non_composed_nodes;
	int i = 0;
	for (auto ni : *nodes)
	{
		from_graph_to_cplex_node_index.insert(pair<unsigned int, unsigned int>(ni, i));
		from_cplex_to_graph_node_index.insert(pair<unsigned int, unsigned int>(i, ni));
		i++;
	}

	long lower_bound = 0;
	IloEnv env;
	try
	{
		IloModel model(env);
//		IloBoolVarArray vars(env, n*n);
		IloNumVarArray vars(env, n*n, 0, 1, ILOFLOAT);

		IloNumExpr iloNumExpr(env);
		unsigned long long _uv;
		unsigned int cg_u, cg_v;
		for (unsigned int u = 0; u < n; u++)
			for (unsigned int v = u + 1; v < n; v++)
			{
				_uv = (unsigned long long)(u) * n + v;
				cg_u = from_cplex_to_graph_node_index.find(u)->second;
				cg_v = from_cplex_to_graph_node_index.find(v)->second;
				if (cg->get_connection_connected_status_from_to(cg_u, cg_v))
					iloNumExpr += (cg->get_weight_between(cg_u, cg_v) * (1 - vars[_uv]));
				else
					iloNumExpr += (-cg->get_weight_between(cg_u, cg_v) * (vars[_uv]));

				if (cg->get_connection_changed_status_from_to(cg_u, cg_v))
					model.add(vars[_uv] == (cg->get_connection_connected_status_from_to(cg_u, cg_v) ? 1 : 0));
			}
		model.add(IloMinimize(env, iloNumExpr));

		unsigned long long vu, vw, uw;
		for (int u = 0; u < n; u++)
			for (int v = 0; v < n; v++)
				for (int w = 0; w < n; w++)
				{
					vu = v * n + u;
					vw = v * n + w;
					uw = u * n + w;
					model.add(vars[vu] + vars[vw] - vars[uw] <= 1);
					model.add(vars[vu] - vars[vw] + vars[uw] <= 1);
					model.add(-vars[vu] + vars[vw] + vars[uw] <= 1);
				}

		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::Param::Simplex::Display, 0);
//		cplex.setParam(IloCplex::Param::Simplex::Tolerances::Markowitz, 0.0001);
		if (!cplex.solve())
		{
			return pair<int, set<pair<int, int>>>(lower_bound, set<pair<int, int>>());
			env.error() << "Failed to optimize LP." << endl;
			throw (-1);
		}

//		IloNumArray vals(env);
//		cplex.getValues(vals, vars);
		lower_bound = cplex.getObjValue();

		/*
			set<pair<int, int>> solution;
			for (int u = 0; u < n; u++)
				for (int v = u + 1; v < n; v++)
				{
					_uv = u * n + v;
					cg_u = from_cplex_to_graph_node_index.find(u)->second;
					cg_v = from_cplex_to_graph_node_index.find(v)->second;
	//				if(vals[_uv] != 0 && vals[_uv] != 1)
	//					int i= 1/0;
					if (cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) < abs(vals[_uv] - 1))
						solution.insert(pair<int, int>(cg_u, cg_v));
					if (!cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) > abs(vals[_uv] - 1) )
						solution.insert(pair<int, int>(cg_u, cg_v));
				}

			return pair<int, set<pair<int, int>>>(cplex.getObjValue(), solution);
		 */
	}
	catch (IloException& e)
	{
		return pair<int, set<pair<int, int>>>(lower_bound, set<pair<int, int>>());
		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...)
	{
		return pair<int, set<pair<int, int>>>(lower_bound, set<pair<int, int>>());
		cerr << "Unknown exception caught" << endl;
	}

	env.end();
	return pair<int, set<pair<int, int>>>(lower_bound, set<pair<int, int>>());
}

pair<int, set<pair<int, int>>> cplex_upper_bound(cluster_graph* cg)
{
	int n = cg->non_composed_nodes.size();
	map<int, int> from_graph_to_cplex_node_index;
	map<int, int> from_cplex_to_graph_node_index;

	set<int> const* nodes = &cg->non_composed_nodes;
	int i = 0;
	for (auto ni : *nodes)
	{
		from_graph_to_cplex_node_index.insert(pair<int, int>(ni, i));
		from_cplex_to_graph_node_index.insert(pair<int, int>(i, ni));
		i++;
	}

	IloEnv env;
	try
	{
		IloModel model(env);
		IloBoolVarArray vars(env, n*n);

//		for (int i = 0; i < n; i++)
//			for (int j = 0; j < n; j++)
//				vars.add(IloNumVar(env, 0.0, 1.0));

		IloNumExpr iloNumExpr(env);
		unsigned long long _uv;
		unsigned cg_u, cg_v;
		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
				cg_u = from_cplex_to_graph_node_index.find(u)->second;
				cg_v = from_cplex_to_graph_node_index.find(v)->second;
				if (cg->get_connection_connected_status_from_to(cg_u, cg_v))
					iloNumExpr += (cg->get_weight_between(cg_u, cg_v) * (1 - vars[_uv]));
				else
					iloNumExpr += (-cg->get_weight_between(cg_u, cg_v) * (vars[_uv]));

//				if(cg->get_connection_changed_status_from_to(cg_u, cg_v))
//					int i=1/0;

				if (cg->get_connection_changed_status_from_to(cg_u, cg_v))
					model.add(vars[_uv] == cg->get_connection_connected_status_from_to(cg_u, cg_v));
			}
		model.add(IloMinimize(env, iloNumExpr));

		unsigned long long vu, vw, uw;
		for (int u = 0; u < n; u++)
			for (int v = 0; v < n; v++)
				for (int w = 0; w < n; w++)
				{
					vu = v * n + u;
					vw = v * n + w;
					uw = u * n + w;
					model.add(vars[vu] + vars[vw] - vars[uw] <= 1);
					model.add(vars[vu] - vars[vw] + vars[uw] <= 1);
					model.add(-vars[vu] + vars[vw] + vars[uw] <= 1);
//					model.add(IloAbs(vars[vu] - 0.5) == 0.5);
//					model.add(IloAbs(vars[vw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[uw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[vw] - 0.5) == 0.5);
//					model.add(IloAbs(vars[uw] - 0.5) == 0.5);
				}

		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::Param::Simplex::Display, 0);
//		cplex.setParam(IloCplex::Param::Simplex::Tolerances::Markowitz, 0.0001);
		if (!cplex.solve())
		{
			env.error() << "Failed to optimize LP." << endl;
			env.end();
			return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
			throw (-1);
		}

		IloNumArray vals(env);
		cplex.getValues(vals, vars);

		set<pair<int, int>> solution;
		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
				cg_u = from_cplex_to_graph_node_index.find(u)->second;
				cg_v = from_cplex_to_graph_node_index.find(v)->second;
//				if(vals[_uv] != 0 && vals[_uv] != 1)
//					int i= 1/0;
				if (cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) < abs(vals[_uv] - 1))
					solution.insert(pair<int, int>(cg_u, cg_v));
				if (!cg->get_connection_connected_status_from_to(cg_u, cg_v) && abs(vals[_uv] - 0) > abs(vals[_uv] - 1) )
					solution.insert(pair<int, int>(cg_u, cg_v));
			}

		int v = cplex.getObjValue();
		env.end();
		return pair<int, set<pair<int, int>>>(v, solution);
	}
	catch (IloException& e)
	{
		env.end();
		return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...)
	{
		env.end();
		return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
		cerr << "Unknown exception caught" << endl;
	}

	env.end();

	return pair<int, set<pair<int, int>>>(-1, set<pair<int, int>>());
}


#endif