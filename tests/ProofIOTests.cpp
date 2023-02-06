

#include "../src/ProofIO/ProofIO.hpp"

int main(){
    try{
        Proof lazyslateBased = ProofIO::loadProof("allRulesProof.json");
        std::cout<< lazyslateBased.assumptions.size()<<std::endl;
        Proof hyperslateBased = ProofIO::loadProof("Branch1-12.slt");
        std::cout<< hyperslateBased.assumptions.size()<<std::endl;
    }catch(std::exception e){
        std::cerr<<e.what();
        return 1;
    }
    
    return 0;
}