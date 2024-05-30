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


unsigned long long count_crossings(unsigned int k, unsigned long long nodes, int node_count,
                             int (*matrix)[32][32]){

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
                                        int (*matrix)[32][32]){

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

int next_in_ordering(std::vector<unsigned long long> *DP, int (*Q)[32][32],
                                    int q){
    //std::vector<unsigned int> ordering;
    unsigned long long mask = (1<<q) - 1;
    int done = -1;

    for (int i = 0; i < q; i++){
        /*
        std::cout << "(*DP)[mask]: " << (*DP)[mask] << "\n";
        std::cout << "(*DP)[mask ^ (1 << i)] + count_crossings: " << (*DP)[mask ^ (1 << i)] + count_crossings(i, (mask ^ (1 << i)), q, Q) <<  "\n";
        std::cout << "(*DP)[mask ^ (1 << i)] " << (*DP)[mask ^ (1 << i)] <<  "\n";
        std::cout << "count_crossings: " << count_crossings(i, (mask ^ (1 << i)), q, Q) <<  "\n";
        */

        if ((*DP)[mask] == (*DP)[mask ^ (1 << i)] + count_crossings(i, (mask ^ (1 << i)), q, Q)){
            return i;
        };
    };
    std::cout << "Got to somewhere we shouldnt be";
    return -1; //Should never reach this stage

    };

std::vector<unsigned int> final_ordering(std::vector<unsigned long long> *DP, int (*matrix)[32][32],
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
        for (int j = num_nodes-1; j >= 0; j--){ 
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
    int S_prime[n];
    int Q[32][32];
    int map_win_to_w[q]; //Keeps track of node-index corresponding to position in W.
    int window[q];
    int s_old;
    int s_new;
    int itteration = 0;
    std::vector<unsigned long long> DP(1<<q);

    for (int i = 0; i < q; i++){ 
        map_win_to_w[i] = S[i];
        window[i] = S[i];
        for (int j = 0; j < q; j++){ //Is this correct????
            Q[i][j] = W[S[i]][S[j]];
        };
    };

    
    while(1){

        //std::cout << "itteration: " << itteration << " \n";
        DP[0] = 0;


        //std::cout << "C_1 \n";

        for (unsigned int l = 1; l < (1<<q); l++){
            DP[l] = cum_crossings(l, q, &DP, &Q);
        };

        
        s_old = next_in_ordering(&DP, &Q, q);
        //std::cout << "C_2 \n";

        S_prime[(n-1) - itteration] = map_win_to_w[s_old];

        //std::cout << "C_3 \n";
        


        if (n-itteration <= q){
            // NEED TO USE FINAL ORDERING AND ADD TO S_PRIME!!!
            for (int i = 0; i < q; i++){ 
                if(i != s_old){
                    S_prime[(n-2) - itteration - i] = map_win_to_w[i];
                };
            };
            break;
        };

        s_new = S[q + itteration];
        window[s_old] = s_new;
        map_win_to_w[s_old] = s_new;

        //std::cout << "C_4 \n";
        for (int i = 0; i < q; i++){ 
            if(s_old != 0){
            //std::cout << "i: " << i << " s_old: " << s_old << "\n";
            //std::cout << "s_new: " << s_new << " map_win_to_w[i]: " << map_win_to_w[i] << "\n";
            }
            Q[s_old][i] = W[s_new][map_win_to_w[i]];    
            Q[i][s_old] = W[map_win_to_w[i]][s_new];    
        };
        itteration += 1;


    }
    //std::cout << "Done" << "\n";

    // NEED TO COUNT HOW MANY LESS/MORE CROSSINGS WE NOW HAVE!
    if (crossings_in_order(S_prime, W, n) < crossings_in_order(S, W, n)){
        std::memcpy(S, S_prime, sizeof(S_prime)); //Replace S by S'
    };

};

void base_heur_test(int seed){
    int n = 94;
    int q = 10;
    int **W;
    int *S;

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
    std::cout << "pre_heru: " << crossings_in_order(S, W, n) << "    ";
    heuristic_dp_ordering(W, S, n, q);
    std::cout << "post_heur: " << crossings_in_order(S, W, n) << "\n";
}

void heur_test(){
    std::cout << "Expected: pre_heru: 39319    post_heur: 39308 \n";
    base_heur_test(2);
    std::cout << "Expected: pre_heru: 39408    post_heur: 39357 \n";
    base_heur_test(3);
    std::cout << "Expected: pre_heru: 39319    post_heur: 39297 \n";
    base_heur_test(4);
    std::cout << "Expected: pre_heru: 39436    post_heur: 39436 \n";
    base_heur_test(5);
    std::cout << "Expected: pre_heru: 39955    post_heur: 39916 \n";
    base_heur_test(6);
    std::cout << "Expected: pre_heru: 39322    post_heur: 39322 \n";
    base_heur_test(7);
    std::cout << "Expected: pre_heru: 39553    post_heur: 39553 \n";
    base_heur_test(8);
};

/* 
 * Finds the minimum feedback vertex set size
 */
int main(int argc, char * argv[]){

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
