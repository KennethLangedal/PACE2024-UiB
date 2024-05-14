#include <fstream>
#include<iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

void runProgram(unsigned int num_nodes, std::string exe_file){

    std::string input_filename = "temp_input.txt";
    std::ofstream input_file(input_filename);
    input_file << num_nodes << std::endl;
    input_file.close();

    std::string command = "./" + exe_file + "< " + input_filename;
    std::system(command.c_str());

    std::remove(input_filename.c_str());
};


int main(){
    for (int i = 15; i <= 24; i++){ 
        runProgram(i, (std::string)"a_impr.out");
    };
};

