#ifndef VERIFIER_H
#define VERIFIER_H

#include <iostream>
#include <sstream>
#include <fstream>

#include "node.h"
#include "cluster_graph.h"

using namespace std;

class verifier
{

public:
    static int binary_search_for_optimal_k(int l, int r, cluster_graph *cg)
    { // r is always considered to be the solution, just not always the optimal
        if (l == r)
            return r;

        int m = (l + r) / 2;
        int ret = cg->solve(m);
        if (ret == -1)
            return binary_search_for_optimal_k(m + 1, r, cg);
        else
        {
            cg->reset_graph(); // resetting graph to starting position after finding solution
            return binary_search_for_optimal_k(l, m, cg);
        }
    }

    static void verify(string file_name, bool with_mergin = true) // file_name is "" if asked to read from cin
    {
        cluster_graph *cg;
        cg = new cluster_graph();
        cg->load_graph(file_name);
        cg->with_merging = true;

        int k;
        for (k = 1; cg->solve(k) == -1; k *= 2)
            ; // if no solution is found, graph remains in original state
        cg->reset_graph();

        k = binary_search_for_optimal_k(k / 2 + 1, k, cg);
        cg->step_count = 0;
        cg->solve(k);
        // cout <<    "-------------------------------------------------\n";
        int changed_edges_cost = 0;
        for (int i = 0; i < cg->n; i++)
        {
            for (int j = i + 1; j < cg->n; j++)
                if (cg->all_edge_statuses[i][j] & 0b100)
                {
                    cout << i + 1 << " " << j + 1 << endl;
                    if ((cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED) && cg->all_nodes[i].connected_nodes.find(node_weight_pair(j, 0)) == cg->all_nodes[i].connected_nodes.end())
                        int i = 1 / 0;
                    if (!(cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED) && cg->all_nodes[i].disconnected_nodes.find(node_weight_pair(j, 0)) == cg->all_nodes[i].disconnected_nodes.end())
                        int i = 1 / 0;
                    changed_edges_cost += (cg->all_edge_statuses[i][j] & cg->CONNECTION_CONNECTED) ? abs(cg->all_nodes[i].connected_nodes.find(node_weight_pair(j, 0))->weight) : abs(cg->all_nodes[i].disconnected_nodes.find(node_weight_pair(j, 0))->weight);
                }
        }
        cout << "#recursive steps: " << cg->step_count << endl;
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
                cout << "verification failed at cost: " << k << ", with edge modified cost: " << changed_edges_cost << ", p3 count: " << p3_count << endl;
        }
    }
};

#endif