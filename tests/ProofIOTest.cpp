
#include <iostream>
#include <cassert>
#include "../src/Proof.hpp"

int main(int argc, char** argv){
    assert(argc == 2);
    std::string fileName = argv[1];
    try{
        Proof proof(fileName);
        std::cout<< proof.toString() << std::endl;
    }catch(std::exception e){
        std::cerr<<e.what()<<std::endl;
        return 1;
    }
    
    return 0;
}