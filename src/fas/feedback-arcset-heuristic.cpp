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

void matrix_32_gen(unsigned int num_nodes, unsigned int (*matrix)[32][32], unsigned int *num_nodes_ref, unsigned int seed){
    unsigned int rand_int;
    *num_nodes_ref = num_nodes;
    if(seed == 0){std::srand(time(NULL));}
    else {std::srand(seed);};
    for (int i = 0; i< num_nodes; i++){
        for (int j = 0; j < num_nodes; j++){
            rand_int = 0 + std::rand() % 10;
            if (!(i == j)){
                (*matrix)[i][j] = rand_int;
            }
        };
    };
}

void matrix_n_gen(unsigned int num_nodes, int **(*matrix), unsigned int seed){
    unsigned int rand_int;
    if(seed == 0){std::srand(time(NULL));}
    else {std::srand(seed);};
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
    std::cout << "Got to somewhere we shouldnt be";
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


void write_results(unsigned int num_nodes, double time){
    std::ofstream outfile;
    outfile.open("log.txt", std::ios_base::app);
    //outfile << "ALGORITHM 2 RESULTS: " << "\n";
    outfile << "number of nodes: " << num_nodes << ", time spent: " << time << std::endl;
};


// main
//
//

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
    /* ALGORITHM:
     * 1. select last q nodes of S, save in qxq-matrix Q.
     * 2. Calculate DP-table of Q, in table DP
     * 3. run final_ordering, only for first element, and prepend corresponding node s_old to new ordering S'
     * 4. Swap s_old for next element s_new in S
     * 5. Swap s_old for s_new in Q
     * 6. repeat until #nodes left in S = q
     * 7. If S' <= S in amount of crossings, set S = S'.
     * 
     * OPTIMIZATIONS:
     * - In 2, if not on first itteration, only update DP-table where s_old was included.
     * - Maybe not remake/make new matrix Q, but have it point to W-values.
     *
     * NOTES:
     * - Currently assuming matrix Q max size is 32x32
     */
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

        q_window[next_swap_idx] = S[remaining_nodes - 1 - q];
        for (int i = 0; i < q; i++){ 
            Q[next_swap_idx][i] = W[q_window[next_swap_idx]][q_window[i]];
            Q[i][next_swap_idx] = W[q_window[i]][q_window[next_swap_idx]];
        };




    };
};

void base_heur_test(int seed, int n, int q){
    int **W;
    int *S;
    int pre, post;

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
   
    /*
     */
    matrix_n_gen(n, &W, seed);

    for (int i = 0; i < n; i++){ 
        S[i] = i;
    };
    
    pre = crossings_in_order(S, W, n);
    std::cout << "pre_heru: " << pre << "    ";

    heuristic_dp_ordering(W, S, n, q);

    post = crossings_in_order(S, W, n);

    std::cout << "post_heur: " << post << "\n";
    std::cout << "change: " << post-pre << "\n";
}

void heur_test(){
    /*
    std::cout << "Expected: pre_heru: 39319    post_heur: 39308 \n";
    base_heur_test(2, 94, 10);
    std::cout << "Expected: pre_heru: 39408    post_heur: 39357 \n";
    base_heur_test(3, 94, 10);
    std::cout << "Expected: pre_heru: 39319    post_heur: 39297 \n";
    base_heur_test(4, 94, 10);
    std::cout << "Expected: pre_heru: 39436    post_heur: 39436 \n";
    base_heur_test(5, 94, 10);
    std::cout << "Expected: pre_heru: 39955    post_heur: 39916 \n";
    base_heur_test(6, 94, 10);
    std::cout << "Expected: pre_heru: 39322    post_heur: 39322 \n";
    base_heur_test(7, 94, 10);
    std::cout << "Expected: pre_heru: 39553    post_heur: 39553 \n";
    */
    base_heur_test(1, 12, 12);
    base_heur_test(1, 14, 12);
    //base_heur_test(8, 94, 10);
    
    //base_heur_test(2, 18, 17);
    //base_heur_test(2, 18, 18);
};

/* 
 * Finds the minimum feedback vertex set size
 */
int main(int argc, char * argv[]){
    std::cout << "Started main\n";
    heur_test();


/*
    auto start_time = std::chrono::high_resolution_clock::now();//timing

    unsigned int num_nodes;
    unsigned int seed;
    num_nodes = atoi(argv[1]);
    seed = atoi(argv[2]);
    unsigned int matrix[32][32];
    matrix_32_gen(num_nodes, &matrix, &num_nodes, seed);

    std::vector<unsigned long long> DP(1<<num_nodes);
    DP[0] = 0;

    for (unsigned int n = 1; n < (1<<num_nodes); n++){
        DP[n] = cum_crossings(n, num_nodes, &DP, &matrix);
    };


    for (unsigned int j : final_ordering(&DP, &matrix, num_nodes)){
        //std::cout << j << " ";
    };


    auto end_time = std::chrono::high_resolution_clock::now();//timing
    double time_spent = ((std::chrono::duration<double, std::ratio<1>>)(end_time - start_time)).count();


    //std::cout << "\n" << DP[(1<<num_nodes) - 1];
    //std::cout << "\n";
    write_results(num_nodes, time_spent);
    */
};
