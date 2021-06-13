#include <ilcplex/ilocplex.h>
#include <cmath>

#include "include/cluster_graph.h"

ILOSTLBEGIN

using namespace std;

int main(int argc, char** argv)
{
	cluster_graph* cg = new cluster_graph();
	string filename = "/home/zia/studies/Algorithm_Engineering/AlgorithmEngineering/wce-students/1-random/r123.dimacs";
//	cg->load_graph(filename);
	cg->load_graph("");
	int n = cg->non_composed_nodes.size();

	IloEnv env;
	try
	{
		IloModel model(env);
		IloNumVarArray vars(env);

//		vars.add(IloNumVar(env, 0.0, 40.0));
//		vars.add(IloNumVar(env));
//		vars.add(IloNumVar(env));

		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				vars.add(IloNumVar(env, 0.0, 1.0));

		IloNumExpr iloNumExpr(env);
		unsigned long long _uv;
		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
				if (cg->get_connection_connected_status_from_to(u, v))
					iloNumExpr += (cg->get_weight_between(u, v) * (1 - vars[_uv]));
				else
					iloNumExpr += (-cg->get_weight_between(u, v) * (vars[_uv]));
			}
		model.add(IloMinimize(env, iloNumExpr));


//		model.add(IloMaximize(env, vars[0] + 2 * vars[1] + 3 * vars[2]));
//		model.add( - vars[0] +     vars[1] + vars[2] <= 20);
//		model.add(   vars[0] - 3 * vars[1] + vars[2] <= 30);

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
					model.add(IloAbs(vars[vu] - 0.5) == 0.5);
					model.add(IloAbs(vars[vw] - 0.5) == 0.5);
					model.add(IloAbs(vars[uw] - 0.5) == 0.5);
				}

		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::Param::Simplex::Display, 0);
//		cplex.setParam(IloCplex::Param::Simplex::Tolerances::Markowitz, 0.0001);
		if (!cplex.solve())
		{
			env.error() << "Failed to optimize LP." << endl;
			throw (-1);
		}

		IloNumArray vals(env);
//		env.out() << "Solution status = " << cplex.getStatus() << endl;
//		env.out() << "Solution value = " << cplex.getObjValue() << endl;
		cplex.getValues(vals, vars);
//		env.out() << "After Values = \n" << vals << endl;

//		int u, v;
//		for (int i = 0; i < vals.getSize(); i++)
//		{
//			u = i / n;
//			v = i % n;
//			if(vals[i] != 0 && vals[i] != 1)
//				int j=1/0;
//			if (u != v)
//			{
//				if (cg->get_connection_connected_status_from_to(u, v) && vals[i] == 0)
//					cout << u+1 << " " << v+1 << endl;
//				if (!cg->get_connection_connected_status_from_to(u, v) && vals[i] == 1)
//					cout << u+1 << " " << v+1 << endl;
//			}
//		}

		for (int u = 0; u < n; u++)
			for (int v = u + 1; v < n; v++)
			{
				_uv = u * n + v;
//				if (vals[_uv] != 0 && vals[_uv] != 1)
//					int j = 1 / 0;
				if (cg->get_connection_connected_status_from_to(u, v) && abs(vals[_uv] - 0) < abs(vals[_uv] - 1))
					cout << u + 1 << " " << v + 1 << endl;
				if (!cg->get_connection_connected_status_from_to(u, v) && abs(vals[_uv] - 1) < abs(vals[_uv] - 0))
					cout << u + 1 << " " << v + 1 << endl;
			}
	}
	catch (IloException& e)
	{
		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...)
	{
		cerr << "Unknown exception caught" << endl;
	}

	env.end();

	return 0;
}