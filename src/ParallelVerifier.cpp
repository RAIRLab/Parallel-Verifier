
#include<mpi.h>
#include<unordered_map>
#include<stdexcept>

#include "LibParallelVerifier/LibParallelVerifier.hpp"
#include "LibParallelVerifier/MPIUtil.hpp"

namespace pv = ParallelVerifier;


//Maps layer mapping algo names to their implementations
using LMMap = std::unordered_map<std::string, LayerMapper>;
const LMMap lmArgMap = {
    {"OzSerial", pv::getLayerMapSerial},
    {"MPI", pv::getLayerMapMPI}
};

//Maps parallel verifier algo names to implementations
using ParallelVerif = bool(*)(const Proof&, LayerMapper);
using PVMap = std::unordered_map<std::string, ParallelVerif>;
const PVMap pvArgMap = {
    {"Original",pv::verifyParallelOriginal},
    {"NoOpt", pv::verifyParallelNoOpt},
    {"LoadBalance", pv::verifyParallelLoadBalance},
};

//Assumes MPI_Init and has already been called
//and stripped other args already. takes in the args and returns 
//which verifier and LayerMapper to use.
std::pair<ParallelVerif, LayerMapper> processArgs(int argc, char** argv){
    if(argc == 2){ //Defaults
        return std::make_pair(pv::verifyParallelNoOpt, pv::getLayerMapMPI); 
    }else if(argc == 4){
        std::string parallelVerifArg = argv[2];
        std::string layerMapperArg = argv[3];
        PVMap::const_iterator pvArgItr = pvArgMap.find(parallelVerifArg);
        LMMap::const_iterator lmArgItr = lmArgMap.find(layerMapperArg);
        if(pvArgItr != pvArgMap.end() && lmArgItr != lmArgMap.end()){
            return std::make_pair(pvArgItr->second, lmArgItr->second); 
        }
    }
    //Failure
    std::cerr<<"Invalid args detected! See Readme for Usage"<<std::endl;
    throw std::runtime_error("Invalid args detected!");
}

int main(int argc, char** argv){
    //MPI Setup
    MPI_Init(&argc, &argv);
    MPIUtil::setGlobals();
    
    //Load Proof
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    std::string proofFileContents = MPIUtil::getFileContents(proofFilePath);
    Proof proof = ProofIO::loadFromJSONContents(proofFileContents);

    //Figure out what algos we're using
    const auto [verifier, layerMapper] = processArgs(argc, argv);

    //Run Verifier
    SharedVerifier::startClock();
    bool result = verifier(proof, layerMapper);
    auto [seconds, cycles] = SharedVerifier::endClock();
    if(myRank == 0){
        std::cout<<seconds<<" Seconds, "<<cycles<<" Clock Cycles"<<std::endl;
    }

    MPI_Finalize();
    return !result;
}