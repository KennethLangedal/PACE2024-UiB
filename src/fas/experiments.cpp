#include <fstream>
#include<iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

void runProgram(unsigned int num_nodes, std::string exe_file, unsigned int seed){

    std::string command = "./" + exe_file + " " + std::to_string(num_nodes) + " " + std::to_string(seed);
    std::system(command.c_str());

};


int main(){
    for (int i = 15; i <= 24; i++){ 
        runProgram(i, (std::string)"a_1.out", 0);
    };

    for (int i = 15; i <= 24; i++){ 
        runProgram(i, (std::string)"a_2.out", 0);
    };
};

