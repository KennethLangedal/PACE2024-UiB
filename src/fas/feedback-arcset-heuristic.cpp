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
#include<fstream>
#include<chrono>
#include<cstdlib>
#include<cstring>

/** 
 * @file  feedback-arcset-dp.cpp
 *
 * @brief calculates the minimum feedback arc set
 *
 * @author Thorgal Blanco
 * Contact: tbl007@uib.no
 */

// Helper functions
//

void matrix_n_gen(unsigned int num_nodes, int **&matrix, unsigned int seed){
    unsigned int rand_int;
    if(seed == 0){std::srand(time(NULL));}
    else {std::srand(seed);};
    for (int i = 0; i< num_nodes; i++){
        for (int j = 0; j < num_nodes; j++){
            rand_int = 0 + std::rand() % 10;
            if (!(i == j)){
                matrix[i][j] = rand_int;
            }
        };
    };
}

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
unsigned long long count_crossings(int k, unsigned long long nodes, int node_count,
                             std::vector<std::vector<int>> &matrix){
    unsigned int count = 0;
    for (unsigned int i = 0; i < node_count; i++){ 
        if (nodes & (1 << i)){
            if ( matrix[k][i] > 0 ){// Checks wether any of the included nodes are in k's adj_list (adj_map)
                count += matrix[k][i];
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

unsigned long long cum_crossings(unsigned long long curr_itt, unsigned int num_nodes, std::vector<unsigned long long> &DP,
                                        std::vector<std::vector<int>> &matrix){

    int sum_initiallized = 0;
    unsigned long long curr_sum;
    unsigned long long curr_perm;
    unsigned long long result;

    for (unsigned int i = 0; i <= std::log2(curr_itt); i++){ 

        if ( curr_itt & (1 << i) ){
            curr_perm = curr_itt;
            curr_perm = curr_perm ^ (unsigned long long) std::pow(2, i); // We exclude curr_node from curr_perm
            curr_sum = DP[curr_perm] + count_crossings(i, curr_perm, num_nodes, matrix);

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

int next_in_ordering(std::vector<unsigned long long> &DP, std::vector<std::vector<int>> &Q,
                                    int q){
    unsigned long long mask = (1<<q) - 1;

    for (int i = 0; i < q; i++){
        
        if (DP[mask] == DP[mask ^ (1 << i)] + count_crossings(i, (mask ^ (1 << i)), q, Q)){
            return i;
        };
    };
    return -1; //Should never reach this stage
};

std::vector<int> final_ordering(std::vector<unsigned long long> &DP, std::vector<std::vector<int>> &matrix,
                                    int num_nodes){
    std::vector<int> ordering;
    unsigned long long mask = (1<<num_nodes) - 1;

    while (mask > 0){
        for (int i = 0; i < num_nodes; i++){
            if ((1 << i) & mask){


                if (DP[mask] == DP[mask ^ (1 << i)] + count_crossings(i, (mask ^ (1 << i)), num_nodes, matrix)){
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

int crossings_in_order(int *S, int **matrix, int num_nodes){
    int count = 0;
    for (int i = num_nodes-1; i >= 0; i--){ 
        for (int j = i - 1; j >= 0; j--){ 
            count += matrix[S[i]][S[j]]; //Crossings from last element to the next
        };
    };
    return count;
};


/* Input: **W matrix of input graph
 *        *S Initial ordering of nodes.
 *        n amount of nodes.
 *        q size of window.
 *
 */
void heuristic_dp_ordering(int **W, int *S, int n, int q){
    int remaining_nodes = n;
    int S_prime[n];
    int q_window[std::min(n, q)];
    int next_swap_idx;
    std::vector<int> last_ordering;
    std::vector<unsigned long long> DP(1<<(std::min(q, n)));
    std::vector<std::vector<int>> Q(32, std::vector<int>(32, 0));

    for (int i = n-1 ; i >= n - std::min(n, q); i--){ 
        q_window[std::min(n, q) - (n - i)] = S[i];
        for (int j = n-1 ; j >= n - std::min(n, q); j--){ 
            Q[std::min(n, q) - (n - i)][std::min(n, q) - (n - j)] = W[S[i]][S[j]];
        };
    };

    while(1){

        DP[0] = 0;

        for (unsigned int l = 1; l < (1<<std::min(q, remaining_nodes)); l++){
            DP[l] = cum_crossings(l, std::min(q, remaining_nodes), DP, Q);
        };


        if (remaining_nodes <= q){
            last_ordering = final_ordering(DP, Q, remaining_nodes);
            for(int i = 0; i < remaining_nodes; i++){
                S_prime[i] = q_window[last_ordering[i]];
            };
            std::memcpy(S, S_prime, n * sizeof(int)); //Replace S by S'

            break; //Algorithm is now done
        };


        remaining_nodes -= 1;
        next_swap_idx = next_in_ordering(DP, Q, q);
        S_prime[remaining_nodes] = q_window[next_swap_idx];

        q_window[next_swap_idx] = S[remaining_nodes - q];
        for (int i = 0; i < q; i++){ 
            Q[next_swap_idx][i] = W[q_window[next_swap_idx]][q_window[i]];
            Q[i][next_swap_idx] = W[q_window[i]][q_window[next_swap_idx]];
        };
    };
};



void heur_test_init(int seed, int n, int q, int **&W, int *&S){

    W = (int **)malloc(n * sizeof(int *));
    S = (int *)malloc(n * sizeof(int));
    if (W == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
    }
    for (int i = 0; i < n; i++) {
        W[i] = (int *)malloc(n * sizeof(int));
        if (W[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
        }
    }
    matrix_n_gen(n, W, seed);

    for (int i = 0; i < n; i++){ 
        S[i] = i;
    };
}
int heur_feasibility_test(){
    int accepted = 1;
    int pre, post;
    int **W;
    int *S;
    int q = 10;
    for (int j = 1; j < 4; j++){ 
        for (int i = 1; i < 100; i+=10){ 
            heur_test_init(j, i, q, W, S);
            pre = crossings_in_order(S, W, i);
            heuristic_dp_ordering(W, S, i, q);
            post = crossings_in_order(S, W, i);
            if ( post - pre > 0 ){accepted = 0;};
            std::sort( S, S + i );
            for (int i = 0; i < i-1; i++){ 
                if( S[i] == S[i+1] || S[i] > i){accepted = 0;};
            };
        };
    };
    return accepted;
};


void heur_test(){
    if ( heur_feasibility_test() == 1 ){
        std::cout << "feasibility test: " << "PASSED" << "\n";
    }
    else{
        std::cout << "feasibility test: " << "FAILED" << "\n";
    };
};

/* 
 * Finds the minimum feedback vertex set size
 */
int main(int argc, char * argv[]){
    std::cout << "Started main\n";
    heur_test();
};
