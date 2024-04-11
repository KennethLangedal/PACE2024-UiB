#include<iostream>
#include<algorithm>
#include<math.h>
#include<vector>
#include<list>
#include<tuple>
#include<unordered_map>
#include<bitset>

/** 
 * @file  feedback-arcset-dp.cpp
 *
 * @brief calculates the minimum feedback arc set
 *
 * @author Thorgal Blanco
 * Contact: tbl007@uib.no
 */



/** TODO:
 * 1. Return the order of nodes, not the sum of edges
 */

//Initialization of graph and DP-table
//
//

unsigned int num_nodes;

std::vector<std::unordered_map<unsigned int, unsigned int>> adj_list;
std::vector<unsigned long long> DP;




// Helper functions
//
//

/* Reads a file and creates the corresponding graph 
 *
 */
void read_input(unsigned int *num_nodes, std::vector<std::unordered_map<unsigned int, unsigned int>> &real_adj_list, 
                 std::vector<unsigned long long> &real_DP){
    unsigned int num_edges;
    std::vector<std::unordered_map<unsigned int, unsigned int>> adj_list;

    std::cin >> *num_nodes;
    std::cin >> num_edges;
    unsigned int out_node;
    unsigned int in_node;
    unsigned int weight;

    std::vector<std::vector<std::tuple<unsigned int, unsigned int>>> placeholder_adj_list(*num_nodes);
    std::vector<unsigned long long> DP((unsigned long long) std::pow(2, *num_nodes));

    for (int i = 0; i < num_edges; i++){ 
        std::cin >> out_node;
        std::cin >> in_node;
        std::cin >> weight;

        placeholder_adj_list[out_node].push_back({in_node, weight});
    };

    std::unordered_map<unsigned int, unsigned int> current_map;

    for (std::vector<std::tuple<unsigned int, unsigned int>> node_vector : placeholder_adj_list){
        current_map.clear();
        for (std::tuple<unsigned int, unsigned int> node_tuple : node_vector){
            current_map.insert(std::make_pair(std::get<0>(node_tuple), std::get<1>(node_tuple)));
        }
        adj_list.push_back(current_map);
    }
    real_adj_list = adj_list;
    real_DP = DP;

};

/* calculates n choose k
 *
 * @param n - number of elements to be distributed
 * @param k - space to distribute the elements
 */
unsigned long long choose(unsigned int n, unsigned int k){

    if ( k > n ) {return 0; };

    if ( k == 0 ) {return 1; };

    unsigned long long mult = n;
    unsigned long long div = k;

    for ( unsigned int i = 1; i < k; i++ ){
        mult *= (n - i);
        div *= (k - i);
    };
    unsigned long long result = mult/div;

    return result;
};

/* Calculates the permutations that contains exactly n 1's and k-n 0's. Used to
 * keep proper ordering during the DP-calculations (Calculate layer by layer)
 *
 * @param n - Amount of 1's (nodes), to be included in the permutation
 * @param k - Total number of nodes (possible 0/1 positions)
 */
std::vector<unsigned long long> get_perms(unsigned int n, unsigned int k){

    std::vector<unsigned long long> result;
    std::vector<unsigned long long> next_perms;
    unsigned long long curr_perm;

    if ( n == 0 ) {return {0}; };

    for ( unsigned int i = 0; i < k; i++){ 
        next_perms = get_perms(n-1, k-1-i);

        for (int n : next_perms){ 
            curr_perm = (n << (1 + i)) | (1 << i);
            result.push_back(curr_perm);
        };
    };

    return result;
};


// Important functions
//
//

/*
 * Calculates the sum of edges we would have to remove if we were to add it to the 
 * DAG containing the optimal ordering of the nodes given in "nodes".
 *
 * @param k - The index of the current node we want to add.
 * @param nodes - Binary encoding of the included nodes we want to compare with.
 * @param node_count - The total amount of nodes in the graph.
 * @param adj_list - The adjecency mapping of the graph
 */

unsigned int count_crossings(unsigned int k, unsigned long long nodes, unsigned int node_count,
                             std::vector<std::unordered_map<unsigned int, unsigned int>> *adj_list){

    unsigned int count = 0;
    std::vector<unsigned int> included_nodes;

    for (unsigned int i = 0; i < node_count; i++){ 
        if (nodes & (1 << i)){
            included_nodes.push_back(i);
        };
    };

    for (unsigned int node : included_nodes){
        if ( ((*adj_list)[k].count(node)) > 0 ){// Checks wether any of the included nodes are in k's adj_list (adj_map)
            count += ((*adj_list)[k][node]);
        };

    };

    return count;
};


/*
 * Given an encoding of the included nodes (curr_itt), it finds the node to add last, that
 * will give the lowest sum of back-nodes given a precomputed best ordering of nodes (DP).
 *
 * @param curr_itt  - Encoding of the included nodes to itterate over
 * @param num_nodes - Amount of nodes in the graph
 * @param *DP       - Pointer to the DP-table
 * @param *adj_list - pointer  to the adjecency mapping
 */

unsigned int cum_crossings(unsigned long long curr_itt, unsigned int num_nodes, std::vector<unsigned long long> *DP,
                            std::vector<std::unordered_map<unsigned int, unsigned int>> *adj_list){

    int sum_initiallized = 0;
    unsigned long long min_sum  = 0;
    unsigned long long curr_perm;

    for (unsigned int i = 0; i < num_nodes; i++){ 

        if ( curr_itt & (1 << i) ){
            curr_perm = curr_itt;
            curr_perm = curr_perm ^ (unsigned long long) std::pow(2, i); // We exclude curr_node from curr_perm

            if (sum_initiallized){// This is the DP recurrsion
                min_sum = std::min(min_sum, (*DP)[curr_perm] + count_crossings(i, curr_perm, num_nodes, adj_list));
            }
            else {min_sum = (*DP)[curr_perm] + count_crossings(i, curr_perm, num_nodes, adj_list);sum_initiallized = 1;};
        };
    };

    return min_sum;

};


// main
//
//

/* 
 * Finds the minimum feedback vertex set size
 */
int main(){

    read_input(&num_nodes, adj_list, DP);

    std::vector<unsigned long long> perms;
    DP[0] = 0;

    for (unsigned int n = 1; n < num_nodes + 1; n++){ 
        perms = get_perms(n, num_nodes);

        for ( unsigned long long curr_itt : perms ){
            DP[curr_itt] = cum_crossings(curr_itt, num_nodes, &DP, &adj_list);
        };
    };

    for (int i = 0; i < std::pow(2, num_nodes); i++){ 
        std::cout << DP[i] << "\n";
    };
};
