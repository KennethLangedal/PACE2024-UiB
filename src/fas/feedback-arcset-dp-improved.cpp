#include<iostream>
#include<algorithm>
#include<math.h>
#include<vector>
#include<list>
#include<tuple>
#include<unordered_map>
#include<bitset>
#include<cstdlib>
#include<time.h>

/** 
 * @file  feedback-arcset-dp.cpp
 *
 * @brief calculates the minimum feedback arc set
 *
 * @author Thorgal Blanco
 * Contact: tbl007@uib.no
 */



/** TODO:
 * 1. Maybe unclude hsum for calculating crossings.
 * 2. PERSONAL: look at perf doe rime checking
 */



// Helper functions
//
//

/* Given a 32x32 matrix, we set its edge-weights to random values for a given amount of nodes.
 * 
 * @param num_nodes         - Amount of nodes the graph has.
 * @param (*matrix)[32][32] - Pointer to the matrix of which values to update.
 */

void matrix_32_gen(unsigned int num_nodes, unsigned int (*matrix)[32][32], unsigned int *num_nodes_ref){
    unsigned int rand_int;
    *num_nodes_ref = num_nodes;
    std::srand(0);
    for (int i = 0; i< num_nodes; i++){
        for (int j = 0; j < num_nodes; j++){
            rand_int = 0 + std::rand() % 10;
            if (!(i == j)){
                (*matrix)[i][j] = rand_int;
            }
        };
    };
}

void initialize_matrix(unsigned int (*matrix)[32][32]){
    for (int i = 0; i < 32; i++){
        for (int j = 0; j < 32; j++){
            (*matrix)[i][j] = 0;
        };
    };
}

void test_matrix_1(unsigned int (*matrix)[32][32], unsigned int *num_nodes){
    *num_nodes = 6;
    initialize_matrix(&(*matrix));
    (*matrix)[0][1] = 3;(*matrix)[1][3] = 2;(*matrix)[1][4] = 4;(*matrix)[2][1] = 10;(*matrix)[2][5] = 4;
    (*matrix)[3][0] = 2;(*matrix)[3][1] = 1;(*matrix)[4][3] = 5;(*matrix)[4][5] = 1;(*matrix)[5][2] = 7;
};

void test_matrix_2(unsigned int (*matrix)[32][32], unsigned int *num_nodes){
    *num_nodes = 10;
    initialize_matrix(&(*matrix));
    (*matrix)[0][1] = 5;(*matrix)[0][2] = 8;
    (*matrix)[0][3] = 3; (*matrix)[1][3] = 8;(*matrix)[2][4] = 5;(*matrix)[3][0] = 1;(*matrix)[3][1] = 7;(*matrix)[4][3] = 26;
    (*matrix)[4][5] = 7;(*matrix)[5][7] = 2;(*matrix)[6][4] = 8;(*matrix)[6][5] = 3;(*matrix)[7][9] = 15;(*matrix)[8][7] = 11;(*matrix)[9][8] = 23;
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

// Important functions
//
//

/*
 * Calculates the sum of edges we would have to remove if we were to add it to the 
 * DAG containing the optimal ordering of the nodes given in "nodes".
 *
 * @param k                 - The index of the current node we want to add.
 * @param nodes             - Binary encoding of the included nodes we want to compare with.
 * @param node_count        - The total amount of nodes in the graph.
 * @param (*matrix)[32][32] - Pointer to the graph-matrix.
 */


unsigned long long count_crossings(unsigned int k, unsigned long long nodes, unsigned int node_count,
                             unsigned int (*matrix)[32][32]){

    unsigned int count = 0;

    for (unsigned int i = 0; i < node_count; i++){ 
        if (nodes & (1 << i)){
            if ( ((*matrix)[k][i]) > 0 ){// Checks wether any of the included nodes are in k's adj_list (adj_map)
                count += ((*matrix)[k][i]);
            };

        };
    };


    return count;
};



/*
 * Given an encoding of the included nodes (curr_itt), it finds the node to add last, that
 * will give the lowest sum of back-nodes given a precomputed best ordering of nodes (DP).
 *
 * @param curr_itt          - Encoding of the included nodes to itterate over
 * @param num_nodes         - Amount of nodes in the graph
 * @param *DP               - Pointer to the DP-table
 * @param (*matrix)[32][32] - pointer  to the graph-matrix
 */

unsigned long long cum_crossings(unsigned long long curr_itt, unsigned int num_nodes, std::vector<unsigned long long> *DP,
                                        unsigned int (*matrix)[32][32]){

    int sum_initiallized = 0;
    unsigned long long curr_sum;
    unsigned long long curr_perm;
    unsigned long long result;

    for (unsigned int i = 0; i <= std::log2(curr_itt); i++){ 

        if ( curr_itt & (1 << i) ){
            curr_perm = curr_itt;
            curr_perm = curr_perm ^ (unsigned long long) std::pow(2, i); // We exclude curr_node from curr_perm
            curr_sum = (*DP)[curr_perm] + count_crossings(i, curr_perm, num_nodes, matrix);


            if (sum_initiallized){// This is the DP recurrsion
                result = std::min(curr_sum, result);
            }
            else {
                result = curr_sum;
                sum_initiallized = 1;
            };
        };
    };

    return result;

};


std::vector<unsigned int> final_ordering(std::vector<unsigned long long> *DP, unsigned int (*matrix)[32][32],
                                    unsigned int num_nodes){
    std::vector<unsigned int> ordering;
    unsigned long long mask = (1<<num_nodes) - 1;

    while (mask > 0){
        for (int i = 0; i < num_nodes; i++){
            if ((1 << i) & mask){

                if ((*DP)[mask] == (*DP)[mask ^ (1 << i)] + count_crossings(i, (mask ^ (1 << i)), num_nodes, matrix)){
                    mask -= (1 << i);
                    ordering.push_back(i);
                    break;
                };
            };
        };
    };
    std::reverse(ordering.begin(), ordering.end());
    return ordering;
};

void write_results(unsigned int *matrix, unsigned int *num_nodes){
};


// main
//
//

/* 
 * Finds the minimum feedback vertex set size
 */
int main(){

    unsigned int num_nodes;
    unsigned int matrix[32][32];
    matrix_32_gen(22, &matrix, &num_nodes);

    std::vector<unsigned long long> DP(1<<num_nodes);
    DP[0] = 0;

    for (unsigned int n = 1; n < (1<<num_nodes); n++){
        DP[n] = cum_crossings(n, num_nodes, &DP, &matrix);
    };



    for (unsigned int j : final_ordering(&DP, &matrix, num_nodes)){
        std::cout << j << " ";
    };

    std::cout << "\n" << DP[(1<<num_nodes) - 1];
    std::cout << "\n";
};
