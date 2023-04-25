
#include "SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier/LibOMPVerifier.hpp"


//Maps parallel verifier algo names to implementations
using OMPVerif = bool(*)(const Proof&);
using PVMap = std::unordered_map<std::string, OMPVerif>;
const PVMap pvArgMap = {
    {"OG", OMPVerifier::OMPVerifyOriginal},
    {"LB", OMPVerifier::OMPVerifyLB},
    {"BF", OMPVerifier::OMPVerifyBF}
};


OMPVerif processArgs(int argc, char** argv){
    if(argc == 2){ //Defaults
        return OMPVerifier::OMPVerifyOriginal; 
    }else if(argc == 3){
        std::string parallelVerifArg = argv[2];
        PVMap::const_iterator pvArgItr = pvArgMap.find(parallelVerifArg);
        if(pvArgItr != pvArgMap.end()){
            return pvArgItr->second; 
        }
    }
    //Failure
    std::cerr<<"Invalid args detected! See Readme for Usage"<<std::endl;
    throw std::runtime_error("Invalid args detected!");
}

int main(int argc, char** argv) {
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof proof(proofFilePath);

    //Figure out which algorithm we're using
    const auto verifier = processArgs(argc, argv);
    
    SharedVerifier::startClock();
    bool result = verifier(proof);
    auto [seconds, cycles] = SharedVerifier::endClock();
    std::cout << seconds << " Seconds, " << cycles << " Clock Cycles";
    std::cout << ", " << !result << " Result Code" << std::endl;
    return !result;
}