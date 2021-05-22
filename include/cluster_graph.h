//
// Created by zia on 4/9/21.
//

#ifndef CLUSTER_GRAPH_H
#define CLUSTER_GRAPH_H

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <sstream>

#include "node.h"
#include "p3.h"

using namespace std;

// converts a graph into a cluster graph with given value of cost if possible
class cluster_graph
{
    friend class verifier;

public:
    int rec_steps = 0;

private:
    node *all_nodes;
    set<p3> p3s_uvw_sorted;
    set<p3> p3s_weight_sorted;
    node *all_nodes_reset;
    set<p3> p3s_uvw_sorted_reset;
    set<p3> p3s_weight_sorted_reset;

    bool are_nodes_connected(int node_index_1, int node_index_2)
    {
        if (all_nodes[node_index_1].connected_nodes.find(node_weight_pair(node_index_2, 0)) !=
            all_nodes[node_index_1].connected_nodes.end())
            return true;
        return false;
    }

    bool are_nodes_disconnected(int node_index_1, int node_index_2)
    {
        if (all_nodes[node_index_1].disconnected_nodes.find(node_weight_pair(node_index_2, 0)) !=
            all_nodes[node_index_1].disconnected_nodes.end())
            return true;
        return false;
    }

    string get_append()
    {
        string append = "\t\t\t";
        string ret = "";
        for (int i = 0; i < rec_steps - 1; i++)
            ret += append;
        return ret;
    }

    void print_graph(string message)
    {
        return;
        cout << "----------------------------------------------------------------------------------\n";
        cout << get_append() << "after: " << message << "\n";
        cout << get_append() << "connections connected\n";
        cout << get_append() << "  ";
        for (int i = 0; i < n + m; i++)
            cout << i << " ";
        cout << endl;
        for (int i = 0; i < n + m; i++)
        {
            cout << get_append() << i << " ";
            for (int j = 0; j < n + m; j++)
                cout << (get_connection_presest_status_from_to(i, j) ? (get_connection_connected_status_from_to(i, j) ? "C" : "D") : "N") << " ";
            cout << endl;
        };

        cout << get_append() << "connection changes\n";
        cout << get_append() << "  ";
        for (int i = 0; i < n + m; i++)
            cout << i << " ";
        cout << endl;
        for (int i = 0; i < n + m; i++)
        {
            cout << get_append() << i << " ";
            for (int j = 0; j < n + m; j++)
                cout << (get_connection_presest_status_from_to(i, j) ? (get_connection_changed_status_from_to(i, j) ? "C" : "S") : "N") << " ";
            cout << endl;
        };

        cout << get_append() << "p3s\n";
        cout << get_append();
        for (set<p3>::iterator i = p3s_uvw_sorted.begin(); i != p3s_uvw_sorted.end(); i++)
            cout << *i << ", ";
        cout << endl;
        cout << "----------------------------------------------------------------------------------\n";
    }

    void disconnect_nodes(int u_index, int v_index, unsigned int after_status)
    {
        node *u_node = &all_nodes[u_index], *v_node = &all_nodes[v_index];
        set<node_weight_pair>::iterator v_node_weight_pair_iterator = u_node->connected_nodes.find(
            node_weight_pair(v_index, 0));
        set<node_weight_pair>::iterator u_node_weight_pair_iterator = v_node->connected_nodes.find(
            node_weight_pair(u_index, 0));
        node_weight_pair v_node_weight_pair = *v_node_weight_pair_iterator;
        node_weight_pair u_node_weight_pair = *u_node_weight_pair_iterator;
        u_node->connected_nodes.erase(v_node_weight_pair_iterator);
        v_node->connected_nodes.erase(u_node_weight_pair_iterator);
        u_node_weight_pair.weight *= -1;
        v_node_weight_pair.weight *= -1;
        u_node->disconnected_nodes.insert(v_node_weight_pair);
        v_node->disconnected_nodes.insert(u_node_weight_pair);

        set<node_weight_pair>::iterator iterator1 = u_node->connected_nodes.begin();
        set<node_weight_pair>::iterator iterator2 = v_node->connected_nodes.begin();

        while (iterator1 != u_node->connected_nodes.end() || iterator2 != v_node->connected_nodes.end())
        {
            if (iterator1 == u_node->connected_nodes.end())
            {
                remove_p3(u_index, v_index, iterator2->node_index);
                iterator2++;
            }
            else if (iterator2 == v_node->connected_nodes.end())
            {
                remove_p3(iterator1->node_index, u_index, v_index);
                iterator1++;
            }
            else if (iterator1->node_index == iterator2->node_index)
            {
                add_p3(u_index, iterator1->node_index, v_index);
                iterator1++;
                iterator2++;
            }
            else if (iterator1->node_index < iterator2->node_index)
            {
                remove_p3(iterator1->node_index, u_index, v_index);
                iterator1++;
            }
            else if (iterator1->node_index > iterator2->node_index)
            {
                remove_p3(u_index, v_index, iterator2->node_index);
                iterator2++;
            }
        }
        all_edge_statuses[u_index][v_index] = after_status;
        all_edge_statuses[v_index][u_index] = after_status;
        print_graph("disconnecting " + to_string(u_index) + " " + to_string(v_index));
    }

    void connect_nodes(int u_index, int v_index, unsigned int after_status)
    {
        node *u_node = &all_nodes[u_index], *v_node = &all_nodes[v_index];
        set<node_weight_pair>::iterator v_node_weight_pair_iterator = u_node->disconnected_nodes.find(
            node_weight_pair(v_index, 0));
        set<node_weight_pair>::iterator u_node_weight_pair_iterator = v_node->disconnected_nodes.find(
            node_weight_pair(u_index, 0));
        node_weight_pair v_node_weight_pair = *v_node_weight_pair_iterator;
        node_weight_pair u_node_weight_pair = *u_node_weight_pair_iterator;
        u_node->disconnected_nodes.erase(v_node_weight_pair_iterator);
        v_node->disconnected_nodes.erase(u_node_weight_pair_iterator);
        u_node_weight_pair.weight *= -1;
        v_node_weight_pair.weight *= -1;
        u_node->connected_nodes.insert(v_node_weight_pair);
        v_node->connected_nodes.insert(u_node_weight_pair);

        set<node_weight_pair>::iterator iterator1 = u_node->connected_nodes.begin();
        set<node_weight_pair>::iterator iterator2 = v_node->connected_nodes.begin();
        while (iterator1 != u_node->connected_nodes.end() || iterator2 != v_node->connected_nodes.end())
        {
            if (iterator1 == u_node->connected_nodes.end())
            {
                if (iterator2->node_index != u_index)
                    add_p3(u_index, v_index, iterator2->node_index);
                iterator2++;
            }
            else if (iterator2 == v_node->connected_nodes.end())
            {
                if (iterator1->node_index != v_index)
                    add_p3(iterator1->node_index, u_index, v_index);
                iterator1++;
            }
            else if (iterator1->node_index == iterator2->node_index)
            {
                remove_p3(u_index, iterator1->node_index, v_index);
                iterator1++;
                iterator2++;
            }
            else if (iterator1->node_index < iterator2->node_index)
            {
                if (iterator1->node_index != v_index)
                    add_p3(iterator1->node_index, u_index, v_index);
                iterator1++;
            }
            else if (iterator1->node_index > iterator2->node_index)
            {
                if (iterator2->node_index != u_index)
                    add_p3(u_index, v_index, iterator2->node_index);
                iterator2++;
            }
        }
        all_edge_statuses[u_index][v_index] = after_status;
        all_edge_statuses[v_index][u_index] = after_status;
        print_graph("connecting " + to_string(u_index) + " " + to_string(v_index));
    }

    void add_p3(int u, int v, int w)
    {
        if (p3s_uvw_sorted.find(p3(u, v, w, 0, p3::UVW_SEARCHABLE)) != p3s_uvw_sorted.end())
            int i = 1 / 0;
        float weight =
            (get_weight_between_nodes(u, v) + get_weight_between_nodes(v, w) +
             abs(get_weight_between_nodes(u, w))) /
            3.0;
        while (p3s_weight_sorted.find(p3(u, v, w, weight, p3::WEIGHT_SEARCHABLE)) != p3s_weight_sorted.end())
            weight += 0.1;

        p3s_uvw_sorted.insert(p3(u, v, w, weight, p3::UVW_SEARCHABLE));
        p3s_weight_sorted.insert(p3(u, v, w, weight, p3::WEIGHT_SEARCHABLE));
    }

    void remove_p3(int u, int v, int w)
    {
        if (p3s_uvw_sorted.find(p3(u, v, w, 0, p3::UVW_SEARCHABLE)) == p3s_uvw_sorted.end())
            int i = 1 / 0;
        float weight = p3s_uvw_sorted.find(p3(u, v, w, 0, p3::UVW_SEARCHABLE))->weight;
        p3s_uvw_sorted.erase(p3(u, v, w, weight, p3::UVW_SEARCHABLE));
        p3s_weight_sorted.erase(p3(u, v, w, weight, p3::WEIGHT_SEARCHABLE));
    }

    // log(n)
    int get_weight_between_nodes(int n1, int n2)
    {
        set<node_weight_pair>::iterator connected = all_nodes[n1].connected_nodes.find(node_weight_pair(n2, 0));
        set<node_weight_pair>::iterator disconnected = all_nodes[n1].disconnected_nodes.find(node_weight_pair(n2, 0));
        if (connected != all_nodes[n1].connected_nodes.end())
            return connected->weight;
        else if (disconnected != all_nodes[n1].disconnected_nodes.end())
            return disconnected->weight;
        return 0;
    }

    void remove_all_p3s_possible_from(int u, bool avoid_p3s_with_merge_partner_index = false)
    {

        int merge_partner_index = -1;
        for (int i = n; i < n + m && merge_partner_index == -1; i++)
            if (all_nodes[i].composed_node_index_1 == u || all_nodes[i].composed_node_index_2 == u)
                merge_partner_index = all_nodes[i].composed_node_index_1 == u ? all_nodes[i].composed_node_index_2 : all_nodes[i].composed_node_index_1;

        node node_u = all_nodes[u];
        set<node_weight_pair>::iterator it_i, it_j;
        for (it_i = all_nodes[u].connected_nodes.begin(); it_i != all_nodes[u].connected_nodes.end(); it_i++)
        {
            node node_i = all_nodes[it_i->node_index];
            for (it_j = all_nodes[it_i->node_index].connected_nodes.begin(); it_j != all_nodes[it_i->node_index].connected_nodes.end(); it_j++)
            {
                node node_j = all_nodes[it_j->node_index];
                if (it_j->node_index != u && (all_nodes[it_j->node_index].composed_node_index_1 != u && all_nodes[it_j->node_index].composed_node_index_2 != u) && !get_connection_connected_status_from_to(u, it_j->node_index))
                    if (!avoid_p3s_with_merge_partner_index || it_i->node_index != merge_partner_index)
                        remove_p3(u, it_i->node_index, it_j->node_index);
            }
        }

        // if (merge_partner_index == -1)
        // int i = 1 / 0;

        for (it_i = all_nodes[u].connected_nodes.begin(); it_i != all_nodes[u].connected_nodes.end(); it_i++)
        {
            it_j = it_i;
            it_j++;
            for (; it_j != all_nodes[u].connected_nodes.end(); it_j++)
                if (it_i->node_index == merge_partner_index ? !get_connection_connected_status_from_to(it_i->node_index, it_j->node_index) : !get_connection_connected_status_from_to(it_j->node_index, it_i->node_index))
                    if (!avoid_p3s_with_merge_partner_index || (it_i->node_index != merge_partner_index && it_j->node_index != merge_partner_index))
                        remove_p3(it_i->node_index, u, it_j->node_index);
        }
    }

    void add_all_p3s_possible_from(int u, bool avoid_p3s_with_merge_partner_index = false)
    {
        int merge_partner_index = -1;
        for (int i = n; i < n + m && merge_partner_index == -1; i++)
            if (all_nodes[i].composed_node_index_1 == u || all_nodes[i].composed_node_index_2 == u)
                merge_partner_index = all_nodes[i].composed_node_index_1 == u ? all_nodes[i].composed_node_index_2 : all_nodes[i].composed_node_index_1;

        set<node_weight_pair>::iterator it_i, it_j;
        for (it_i = all_nodes[u].connected_nodes.begin(); it_i != all_nodes[u].connected_nodes.end(); it_i++)
            for (it_j = all_nodes[it_i->node_index].connected_nodes.begin(); it_j != all_nodes[it_i->node_index].connected_nodes.end(); it_j++)
                if (it_j->node_index != u && !get_connection_connected_status_from_to(it_j->node_index, u))
                    if (!avoid_p3s_with_merge_partner_index || it_i->node_index != merge_partner_index)
                        add_p3(u, it_i->node_index, it_j->node_index);

        for (it_i = all_nodes[u].connected_nodes.begin(); it_i != all_nodes[u].connected_nodes.end(); it_i++)
        {
            it_j = it_i;
            it_j++;
            for (; it_j != all_nodes[u].connected_nodes.end(); it_j++)
                if (!get_connection_connected_status_from_to(it_i->node_index, it_j->node_index))
                    if (!avoid_p3s_with_merge_partner_index || (it_i->node_index != merge_partner_index && it_j->node_index != merge_partner_index))
                        add_p3(it_i->node_index, u, it_j->node_index);
        }
    }

    bool get_connection_presest_status_from_to(int f, int t)
    {
        return all_edge_statuses[f][t] & CONNECTION_PRESENT;
    }

    bool get_connection_connected_status_from_to(int f, int t)
    {
        if (!get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;
        return all_edge_statuses[f][t] & CONNECTION_CONNECTED;
    }

    bool get_connection_changed_status_from_to(int f, int t)
    {
        if (!get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;
        return all_edge_statuses[f][t] & CONNECTION_CHANGED;
    }

    bool are_non_composed_nodes(int f, int t) // checking if on same merge level
    {
        return get_connection_presest_status_from_to(f, t) && get_connection_presest_status_from_to(t, f);
    }

    int get_weight_between(int u, int v)
    {
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;
        return (get_connection_connected_status_from_to(u, v) ? all_nodes[u].connected_nodes : all_nodes[u].disconnected_nodes).find(node_weight_pair(v, 0))->weight;
    }

    int get_weight_from_to(int f, int t) // should be used with care as it can break graph rules as it is only made for demerging
    {
        if (!get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;

        return (get_connection_connected_status_from_to(f, t) ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).find(node_weight_pair(t, 0))->weight;
    }

    unsigned int get_status_from_to(int f, int t)
    {
        return all_edge_statuses[f][t];
    }

    void remove_connection_from_to(int f, int t, bool without_checking_connection = false)
    {
        if (!without_checking_connection && !are_non_composed_nodes(f, t))
            int i = 1 / 0;
        (get_connection_connected_status_from_to(f, t) ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).erase(node_weight_pair(t, 0));
        all_edge_statuses[f][t] = 0;
    }

    void remove_connection_between(int u, int v)
    {
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;
        remove_connection_from_to(u, v, true);
        remove_connection_from_to(v, u, true);
    }

    void add_connection_from_to(int f, int t, int cost, unsigned int status_ft)
    {
        if (get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;
        if ((cost > 0 && !(status_ft & CONNECTION_CONNECTED)))
            int i = 1 / 0;
        if ((cost < 0 && (status_ft & CONNECTION_CONNECTED)))
            int i = 1/0;

            all_edge_statuses[f][t] = status_ft;
        (status_ft & CONNECTION_CONNECTED ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).insert(node_weight_pair(t, cost));
    }

    void add_connection_between(int u, int v, int cost, unsigned int status_uv)
    {
        add_connection_from_to(u, v, cost, status_uv);
        add_connection_from_to(v, u, cost, status_uv);
    }

    void update_connection_from_to(int f, int t, int cost, unsigned int status_ft)
    {
        if (!get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;

        remove_connection_from_to(f, t, true);
        add_connection_from_to(f, t, cost, status_ft);
    }

    void update_connection_between(int f, int t, int cost, unsigned int status_ft)
    {
        update_connection_from_to(f, t, cost, status_ft);
        update_connection_from_to(t, f, cost, status_ft);
    }

    void flip_connection_from_to(int f, int t)
    {
        if (!get_connection_presest_status_from_to(f, t))
            int i = 1 / 0;
        if(get_connection_changed_status_from_to(f, t))
            int i= 1/0;

        bool connection_status = get_connection_connected_status_from_to(f, t);

        if ((connection_status ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).find(node_weight_pair(t, 0)) ==
            (connection_status ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).end())
            int i = 1 / 0;

        node_weight_pair pair = *((connection_status ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).find(node_weight_pair(t, 0)));

        (connection_status ? all_nodes[f].connected_nodes : all_nodes[f].disconnected_nodes).erase(pair);
        (connection_status ? all_nodes[f].disconnected_nodes : all_nodes[f].connected_nodes).insert(pair);

        all_edge_statuses[f][t] = CONNECTION_PRESENT | (connection_status ? 0 : CONNECTION_CONNECTED) | CONNECTION_CHANGED;
    }

    void flip_connection_between(int u, int v)
    {
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;

        flip_connection_from_to(u, v);
        flip_connection_from_to(v, u);
    }

    enum merge_result
    {
        TOO_EXPENSIVE,
        NOT_POSSIBLE_EDGES_MODIFIED,
        POSSIBLE_WITH_COST
    };
    pair<merge_result, int> merge_(int u, int v, int budget)
    {
        int cost_ui, cost_vi;
        unsigned int status_ui, status_vi;
        bool atleast_one_unchanged;
        bool same_connected_state_ui_vi;
        int min_cost, total_cost_of_merge = 0;

        for (int i = 0; i < n + m; i++)
            if (i != u && i != v && are_non_composed_nodes(u, i) && are_non_composed_nodes(v, i))
            {
                cost_ui = get_weight_between(u, i);
                cost_vi = get_weight_between(v, i);
                status_ui = get_status_from_to(u, i);
                status_vi = get_status_from_to(v, i);
                atleast_one_unchanged = !(status_ui & CONNECTION_CHANGED) || !(status_vi & CONNECTION_CHANGED);
                same_connected_state_ui_vi = (status_ui & CONNECTION_CONNECTED) == (status_vi & CONNECTION_CONNECTED);

                if ((cost_ui * cost_vi == 0 && !atleast_one_unchanged && !same_connected_state_ui_vi) || (cost_ui * cost_vi < 0 && !atleast_one_unchanged))
                    return pair<merge_result, int>(NOT_POSSIBLE_EDGES_MODIFIED, -1);

                if (cost_ui * cost_vi <= 0)
                {
                    min_cost = INT32_MAX;
                    if (!(status_ui & CONNECTION_CHANGED))
                        min_cost = min_cost < abs(cost_ui) ? min_cost : abs(cost_ui);
                    if (!(status_vi & CONNECTION_CHANGED))
                        min_cost = min_cost < abs(cost_vi) ? min_cost : abs(cost_vi);

                    total_cost_of_merge += min_cost;
                }
            }

        if (total_cost_of_merge > budget)
            return pair<merge_result, int>(TOO_EXPENSIVE, -1);

        int _m = n + m;
        m++;
        if (m >= 2 * n)
            int i = 1 / 0;

        all_nodes[_m].composed_node_index_1 = u;
        all_nodes[_m].composed_node_index_2 = v;
        remove_all_p3s_possible_from(u);
        remove_all_p3s_possible_from(v, true);
        remove_connection_from_to(v, u);

        int cost_m;
        unsigned int status_m;
        unsigned int status_zero, status_non_zero;
        int cost_non_zero;
        int abs_cost;
        bool both_unchanged;

        for (int i = 0; i < n + m - 1; i++)
            if (i != u && i != v && are_non_composed_nodes(u, i) && are_non_composed_nodes(v, i))
            {
                cost_ui = get_weight_between(u, i);
                cost_vi = get_weight_between(v, i);
                status_ui = get_status_from_to(u, i);
                status_vi = get_status_from_to(v, i);
                atleast_one_unchanged = !(status_ui & CONNECTION_CHANGED) || !(status_vi & CONNECTION_CHANGED);
                both_unchanged = !(status_ui & CONNECTION_CHANGED) && !(status_vi & CONNECTION_CHANGED);
                same_connected_state_ui_vi = (status_ui & CONNECTION_CONNECTED) == (status_vi & CONNECTION_CONNECTED);

                abs_cost = abs(cost_ui) + abs(cost_vi);

                if (cost_ui * cost_vi == 0)
                {
                    if (atleast_one_unchanged)
                    {
                        status_zero = cost_ui == 0 ? status_ui : status_vi;
                        status_non_zero = cost_ui != 0 ? status_ui : status_vi;
                        cost_non_zero = cost_ui != 0 ? cost_ui : cost_vi;

                        if (!(status_zero & CONNECTION_CHANGED))
                            status_m = status_non_zero;
                        else if (status_zero & CONNECTION_CHANGED)
                            status_m = status_zero;
                    }
                    else if (same_connected_state_ui_vi)
                        status_m = CONNECTION_PRESENT | (status_ui & CONNECTION_CONNECTED) | CONNECTION_CHANGED;

                    cost_m = !(status_m & CONNECTION_CHANGED) ? cost_ui + cost_vi : (status_m & CONNECTION_CONNECTED ? abs_cost : -abs_cost);
                }
                else if (cost_ui * cost_vi > 0)
                {
                    status_m = CONNECTION_PRESENT | (cost_ui > 0 ? CONNECTION_CONNECTED : 0) | (both_unchanged ? 0 : CONNECTION_CHANGED);
                    cost_m = cost_ui + cost_vi;
                }
                else if (cost_ui * cost_vi < 0)
                {
                    if (both_unchanged)
                    {
                        status_m = CONNECTION_PRESENT | (cost_ui + cost_vi >= 0 ? CONNECTION_CONNECTED : 0) | 0;
                        cost_m = cost_ui + cost_vi;
                    }
                    else if (atleast_one_unchanged)
                    {
                        status_m = status_ui & CONNECTION_CHANGED ? status_ui : status_vi;
                        cost_m = (status_ui & CONNECTION_CHANGED ? 1 : -1) * cost_ui + (status_vi & CONNECTION_CHANGED ? 1 : -1) * cost_vi;
                    }
                }

                remove_connection_from_to(i, u);
                remove_connection_from_to(i, v);
                add_connection_between(_m, i, cost_m, status_m);
            }

        add_all_p3s_possible_from(_m);
        print_graph("merging " + to_string(u) + " " + to_string(v));
        return pair<merge_result, int>(POSSIBLE_WITH_COST, total_cost_of_merge);
    }

    pair<merge_result, int> merge(int u, int v, int budget)
    {
        bool connection_changed_status_ui;
        bool connection_changed_status_vi;
        bool connectivity_status_ui;
        bool connectivity_status_vi;
        int cost_ui;
        int cost_vi;
        bool atleast_one_unchanged;
        int merge_cost = 0;
        int min_merge_cost_i;
        bool both_unchanged;
        for (int i = 0; i < n + m; i++)
            if (i != u && i != v && are_non_composed_nodes(u, i) && are_non_composed_nodes(v, i))
            {
                connection_changed_status_ui = get_connection_changed_status_from_to(u, i);
                connection_changed_status_vi = get_connection_changed_status_from_to(v, i);
                connectivity_status_ui = get_connection_connected_status_from_to(u, i);
                connectivity_status_vi = get_connection_connected_status_from_to(v, i);
                cost_ui = get_weight_between(u, i);
                cost_vi = get_weight_between(v, i);
                atleast_one_unchanged = !connection_changed_status_ui || !connection_changed_status_vi;
                both_unchanged = !connection_changed_status_ui && !connection_changed_status_vi;

                // if(!both_unchanged && atleast_one_unchanged && (connectivity_status_ui != connectivity_status_vi))
                if(!both_unchanged)
                        return pair<merge_result, int>(NOT_POSSIBLE_EDGES_MODIFIED, -1);

                if (connectivity_status_ui != connectivity_status_vi)
                {
                    if (atleast_one_unchanged)
                    {
                        min_merge_cost_i = INT32_MAX;
                        if (!connection_changed_status_ui)
                            min_merge_cost_i = min_merge_cost_i < abs(cost_ui) ? min_merge_cost_i : abs(cost_ui);
                        if (!connection_changed_status_vi)
                            min_merge_cost_i = min_merge_cost_i < abs(cost_vi) ? min_merge_cost_i : abs(cost_vi);
                        if (min_merge_cost_i == INT32_MAX)
                            int i = 1 / 0;
                        merge_cost += min_merge_cost_i != INT32_MAX ? min_merge_cost_i : 0;
                    }
                    else
                        return pair<merge_result, int>(NOT_POSSIBLE_EDGES_MODIFIED, -1);
                }
            }

        if (min_merge_cost_i > budget)
            return pair<merge_result, int>(TOO_EXPENSIVE, -1);

        if (m >= n)
            int i = 1 / 0;
        int _m = n + m;
        m++;

        all_nodes[_m].composed_node_index_1 = u;
        all_nodes[_m].composed_node_index_2 = v;
        remove_all_p3s_possible_from(u);
        remove_all_p3s_possible_from(v, true);
        remove_connection_from_to(v, u);

        bool same_connectivity;
        unsigned int status_mi;
        int cost_mi;
        for (int i = 0; i < n + m - 1; i++)
            if (i != u && i != v && are_non_composed_nodes(u, i) && are_non_composed_nodes(v, i))
            {
                connection_changed_status_ui = get_connection_changed_status_from_to(u, i);
                connection_changed_status_vi = get_connection_changed_status_from_to(v, i);
                connectivity_status_ui = get_connection_connected_status_from_to(u, i);
                connectivity_status_vi = get_connection_connected_status_from_to(v, i);
                both_unchanged = !connection_changed_status_ui && !connection_changed_status_vi;
                atleast_one_unchanged = !connection_changed_status_ui || !connection_changed_status_vi;
                same_connectivity = connectivity_status_ui == connectivity_status_vi;
                cost_ui = get_weight_between(u, i);
                cost_vi = get_weight_between(v, i);

                if (both_unchanged)
                {
                    if (same_connectivity)
                    {
                        cost_mi = cost_ui + cost_vi;
                        status_mi = CONNECTION_PRESENT | (connectivity_status_ui ? CONNECTION_CONNECTED : 0) | 0;
                    }
                    else
                    {
                        cost_mi = cost_ui + cost_vi;
                        status_mi = CONNECTION_PRESENT |
                                    ((abs(cost_ui) > abs(cost_vi) ? connectivity_status_ui : connectivity_status_vi) ? CONNECTION_CONNECTED : 0) | 0;
                    }
                }
                else if (atleast_one_unchanged)
                {
                    if (same_connectivity)
                    {
                        cost_mi = cost_ui + cost_vi;
                        status_mi = CONNECTION_PRESENT | (connectivity_status_ui ? CONNECTION_CONNECTED : 0) | CONNECTION_CHANGED;
                    }
                    else
                    {
                        cost_mi = connection_changed_status_ui ? cost_ui : cost_vi;
                        status_mi = CONNECTION_PRESENT | ((connection_changed_status_ui ? connectivity_status_ui : connectivity_status_vi) ? CONNECTION_CONNECTED : 0) | CONNECTION_CHANGED;
                    }
                }
                else
                {
                    if (same_connectivity)
                    {
                        cost_mi = cost_ui + cost_vi;
                        status_mi = CONNECTION_PRESENT | (connectivity_status_ui ? CONNECTION_CONNECTED : 0) | CONNECTION_CHANGED;
                    }
                    else
                        int i = 1 / 0;
                }

                remove_connection_from_to(i, u);
                remove_connection_from_to(i, v);
                add_connection_between(_m, i, cost_mi, status_mi);
            }

        add_all_p3s_possible_from(_m);

        return pair<merge_result, int>(POSSIBLE_WITH_COST, merge_cost);
    }

    void demerge(bool solution_found)
    {
        if (m == 0)
            int i = 1 / 0;
        int _m = n + m - 1;
        int u = all_nodes[_m].composed_node_index_1;
        int v = all_nodes[_m].composed_node_index_2;
        unsigned int status_mi, status_ui, status_vi;
        int cost_ui, cost_vi;
        node node_u = all_nodes[u];
        node node_v = all_nodes[v];
        remove_all_p3s_possible_from(_m);
        add_connection_from_to(v, u, get_weight_from_to(u, v), get_status_from_to(u, v));

        for (int i = 0; i < n + m; i++)
            if (are_non_composed_nodes(_m, i))
            {
                status_mi = get_status_from_to(_m, i);
                status_ui = get_status_from_to(u, i);
                status_vi = get_status_from_to(v, i);
                cost_ui = get_weight_from_to(u, i);
                cost_vi = get_weight_from_to(v, i);

                remove_connection_between(_m, i);
                add_connection_from_to(i, u, cost_ui, status_ui);
                add_connection_from_to(i, v, cost_vi, status_vi);

                if (solution_found)
                {
                    if ((status_mi & CONNECTION_CONNECTED) != (status_ui & CONNECTION_CONNECTED))
                        flip_connection_between(u, i);

                    if ((status_mi & CONNECTION_CONNECTED) != (status_vi & CONNECTION_CONNECTED))
                        flip_connection_between(v, i);
                }
            }

        add_all_p3s_possible_from(u);
        add_all_p3s_possible_from(v, true);

        all_nodes[_m].composed_node_index_1 = -1;
        all_nodes[_m].composed_node_index_2 = -1;
        m--;

        print_graph("demerging " + to_string(u) + " " + to_string(v));
    }

public:
    enum edge_status : unsigned int // first bit is connection_present, second bit is connection type(connected/disconnected), third bit is connection state(changed/unchanged)
    {
        CONNECTION_PRESENT = 0b001,
        CONNECTION_CONNECTED = 0b010,
        CONNECTION_CHANGED = 0b100,
    };

    int n;
    int m;
    unsigned int **all_edge_statuses;

    cluster_graph() : n(-1), all_nodes(NULL), all_nodes_reset(NULL), all_edge_statuses(NULL) {}

    void load_graph(string file_name) // file_name is "" if asked to read from cin
    {
        istream *input_stream = NULL;
        ifstream fin(file_name);
        if (file_name != "")
            input_stream = dynamic_cast<istream *>(&fin);
        else
            input_stream = dynamic_cast<istream *>(&cin);

        *input_stream >> n;

        p3::set_n(2 * n);
        if (all_nodes != NULL)
            delete[] all_nodes; // delete old nodes;
        all_nodes = new node[2 * n];

        if (all_edge_statuses == NULL)
        {
            all_edge_statuses = new unsigned int *[2 * n];
            for (int i = 0; i < 2 * n; i++)
                all_edge_statuses[i] = new unsigned int[2 * n];
        }

        for (int i = 0; i < 2 * n; i++)
            for (int j = 0; j < 2 * n; j++)
                all_edge_statuses[i][j] = 0;

        m = 0;

        string line;
        int u, v, w;
        for (int i = 0; i < ((n - 1) * n) / 2;)
        {
            if (input_stream != &cin && input_stream->eof())
                exit(-1); // if reading from file check eof

            getline(*input_stream, line);

            if (line != "")
            {
                stringstream ss(line);
                ss >> u >> v >> w;
                if (w > 0)
                {
                    all_nodes[u - 1].connected_nodes.insert(node_weight_pair(v - 1, w));
                    all_nodes[v - 1].connected_nodes.insert(node_weight_pair(u - 1, w));
                }
                else if (w < 0)
                {
                    all_nodes[u - 1].disconnected_nodes.insert(node_weight_pair(v - 1, w));
                    all_nodes[v - 1].disconnected_nodes.insert(node_weight_pair(u - 1, w));
                }
                all_edge_statuses[u - 1][v - 1] = CONNECTION_PRESENT | (w > 0 ? CONNECTION_CONNECTED : 0) | 0;
                all_edge_statuses[v - 1][u - 1] = CONNECTION_PRESENT | (w > 0 ? CONNECTION_CONNECTED : 0) | 0;
                i++;
            }
        }

        if (file_name != "")
        {
            fin.close();
            input_stream = NULL;
        }

        node *v_node;
        set<node_weight_pair>::iterator it1, it2;
        for (int i = 0; i < n; i++)
        {
            v_node = &all_nodes[i];
            for (it1 = v_node->connected_nodes.begin(); it1 != v_node->connected_nodes.end(); it1++)
            {
                it2 = it1;
                it2++;
                for (; it2 != v_node->connected_nodes.end(); it2++)
                    if (are_nodes_disconnected(it1->node_index, it2->node_index))
                        add_p3(it1->node_index, i, it2->node_index);
            }
        }

        if (all_nodes_reset != NULL)
            delete[] all_nodes_reset; // deleting old backup
        all_nodes_reset = new node[2 * n];

        p3s_uvw_sorted_reset = p3s_uvw_sorted;
        p3s_weight_sorted_reset = p3s_weight_sorted;
        for (int i = 0; i < 2 * n; i++)
            all_nodes_reset[i] = all_nodes[i];
    }

    int try_merge(int budget)
    {
        pair<merge_result, int> result;
        for (int i = 0; i < n + m; i++)
            for (int j = i + 1; j < n + m; j++)
            {
                if (are_non_composed_nodes(i, j) && ((get_weight_between(i, j) > budget && get_connection_connected_status_from_to(i, j)) || merge_reduction_rule_2(i, j) || merge_reduction_rule_1(i, j)))
                {
                    result = merge(i, j, budget);
                    if (result.first == POSSIBLE_WITH_COST)
                        return result.second;
                }
            }
        return -1;
    }

    int step_count;
    bool with_merging = false;
    // O(3^k*n*log(n)), k is budget, n is no. of nodes.
    int solve(int budget)
    {
        step_count++;
        if (budget < 0)
            return -1;

        if (p3s_uvw_sorted.empty())
            return budget;

        rec_steps++;
        // int k;
        // int previous_merge_nodes = m;
        // while (with_merging)
        // {
        //     k = try_merge(budget);
        //     if (k != -1)
        //         budget -= k;
        //     else
        //         break;
        // }

        if (p3s_uvw_sorted.empty())
        {
            // while (m != previous_merge_nodes)
                // demerge(true);
            rec_steps--;
            return budget;
        }

        set<p3>::iterator iterator = p3s_weight_sorted.begin();
        int u_index = iterator->u;
        int v_index = iterator->v;
        int w_index = iterator->w;

        if (!(all_edge_statuses[v_index][u_index] & CONNECTION_CHANGED))
        {
            disconnect_nodes(v_index, u_index, CONNECTION_PRESENT | 0 | CONNECTION_CHANGED);

            int budget_left = budget - abs(all_nodes[v_index].disconnected_nodes.find(
                                                                                    node_weight_pair(u_index, 0))
                                               ->weight);
            budget_left = solve(budget_left);
            if (budget_left != -1)
            {
                // while (m != previous_merge_nodes)
                    // demerge(true);
                rec_steps--;
                return budget_left;
            }

            connect_nodes(v_index, u_index, CONNECTION_PRESENT | CONNECTION_CONNECTED | 0);
        }
        if (!(all_edge_statuses[v_index][w_index] & CONNECTION_CHANGED))
        {
            disconnect_nodes(v_index, w_index, CONNECTION_PRESENT | 0 | CONNECTION_CHANGED);

            int budget_left = budget - abs(all_nodes[v_index].disconnected_nodes.find(
                                                                                    node_weight_pair(w_index, 0))
                                               ->weight);
            budget_left = solve(budget_left);
            if (budget_left != -1)
            {

                // while (m != previous_merge_nodes)
                    // demerge(true);
                rec_steps--;
                return budget_left;
            }

            connect_nodes(v_index, w_index, CONNECTION_PRESENT | CONNECTION_CONNECTED | 0);
        }
        if (!(all_edge_statuses[u_index][w_index] & CONNECTION_CHANGED) && data_reduction_rules(u_index, w_index))
        {
            connect_nodes(u_index, w_index, CONNECTION_PRESENT | CONNECTION_CONNECTED | CONNECTION_CHANGED);

            int budget_left = budget - abs(all_nodes[u_index].connected_nodes.find(
                                                                                 node_weight_pair(w_index, 0))
                                               ->weight);

            pair<merge_result, int> result;
            result = merge(u_index, w_index, budget_left);
            if (result.first == POSSIBLE_WITH_COST)
                budget_left -= result.second;

            budget_left = solve(budget_left);

            if (budget_left != -1)
            {
                // while (m != previous_merge_nodes)
                    // demerge(true);
                if(result.first == POSSIBLE_WITH_COST)
                    demerge(true);
                rec_steps--;
                return budget_left;
            }
            if(result.first == POSSIBLE_WITH_COST)
                demerge(false);

            disconnect_nodes(u_index, w_index, CONNECTION_PRESENT | 0 | 0);
        }
        // while (m != previous_merge_nodes)
            // demerge(false);
        rec_steps--;
        return -1;
    }

    bool data_reduction_rules(int u, int v)
    {
        // return true;
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;

        if (!get_connection_connected_status_from_to(u, v))
        {
            int sum = 0;
            for (auto i = all_nodes[u].connected_nodes.begin(); i != all_nodes[u].connected_nodes.end(); i++)
                if (i->node_index != v)
                    sum += (*i).weight;

            if (sum <= abs(get_weight_between(u, v)))
                return false;

            sum = 0;
            for (auto i = all_nodes[v].connected_nodes.begin(); i != all_nodes[v].connected_nodes.end(); i++)
                if (i->node_index != u)
                    sum += (*i).weight;

            if (sum <= abs(get_weight_between(u, v)))
                return false;
        }

        return true;
    }

    bool merge_reduction_rule_1(int u, int v)
    {
        return false;
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;

        int sum = 0;
        if (get_connection_connected_status_from_to(u, v))
        {
            for (auto i = all_nodes[u].connected_nodes.begin(); i != all_nodes[u].connected_nodes.end(); i++)
                if (i->node_index != v)
                    sum += i->weight;
            for (auto i = all_nodes[u].disconnected_nodes.begin(); i != all_nodes[u].disconnected_nodes.end(); i++)
                if (i->node_index != v)
                    sum += abs(i->weight);

            if (sum <= get_weight_between(u, v))
                return true;
        }

        return false;
    }

    bool merge_reduction_rule_2(int u, int v)
    {
        if (!are_non_composed_nodes(u, v))
            int i = 1 / 0;

        int sum = 0;
        if (get_connection_connected_status_from_to(u, v))
        {
            for (auto i = all_nodes[u].connected_nodes.begin(); i != all_nodes[u].connected_nodes.end(); i++)
                if (i->node_index != v)
                    sum += (*i).weight;

            for (auto i = all_nodes[v].connected_nodes.begin(); i != all_nodes[v].connected_nodes.end(); i++)
                if (i->node_index != u)
                    sum += (*i).weight;

            if (sum <= get_weight_between(u, v))
                return true;
        }

        return false;
    }

    void reset_graph()
    {
        rec_steps = 0;
        m = 0;
        p3s_uvw_sorted = p3s_uvw_sorted_reset;
        p3s_weight_sorted = p3s_weight_sorted_reset;
        for (int i = 0; i < 2 * n; i++)
            all_nodes[i] = all_nodes_reset[i];

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                all_edge_statuses[i][j] = CONNECTION_PRESENT | (all_nodes[i].connected_nodes.find(node_weight_pair(j, 0)) != all_nodes[i].connected_nodes.end() ? CONNECTION_CONNECTED : 0) | 0;

        for (int i = n; i < 2 * n; i++)
            for (int j = n; j < 2 * n; j++)
                all_edge_statuses[i][j] = 0;
    }
};

#endif