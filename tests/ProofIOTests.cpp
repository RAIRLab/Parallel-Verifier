

#include "../src/Proof.hpp"

int main(){
    try{
        Proof lazyslateBased("allRulesProof.json");
        std::cout<< lazyslateBased.assumptions.size()<<std::endl;
        Proof hyperslateBased("Branch1-12.slt");
        std::cout<< hyperslateBased.assumptions.size()<<std::endl;
    }catch(std::exception e){
        std::cerr<<e.what()<<std::endl;
        return 1;
    }
    
    return 0;
}