
#include<mpi.h>
#include<unordered_map>

#include "LibParallelVerifier/LibParallelVerifier.hpp"
#include "LibParallelVerifier/MPIUtil.hpp"

namespace pv = ParallelVerifier;



//Maps layer mapping algo names to their implementations
using LMMap = std::unordered_map<std::string, LayerMapper>;
const LMMap lmArgMap = {
    {"Serial", pv::getLayerMapSerial},
    {"MPI", pv::getLayerMapMPI}
};

//Maps parallel verifier algo names to implementations
using ParallelVerif = bool(*)(const Proof&, LayerMapper);
using PVMap = std::unordered_map<std::string, ParallelVerif>;
const PVMap pvArgMap = {
    {"NoOpt", pv::verifyParallelNoOpt},
    {"LoadBalance", pv::verifyParallelLoadBalance},
    {"SemanticJump",pv::verifyParallelSemanticJump}
};

int main(int argc, char** argv){
    //MPI Setup
    MPI_Init(&argc, &argv);
    MPIUtil::setGlobals();
    
    //Load Proof
    const char* proofFilePath = SharedVerifier::init(&argc, &argv);
    std::string proofFileContents = MPIUtil::getFileContents(proofFilePath);
    Proof proof = ProofIO::loadFromJSONContents(proofFileContents);

    //Run Verifier
    SharedVerifier::startClock();
    bool result = pv::verifyParallelNoOpt(proof, pv::getLayerMapMPI);
    auto [seconds, cycles] = SharedVerifier::endClock();
    if(myRank == 0){
        std::cout<<seconds<<" Seconds, "<<cycles<<" Clock Cycles"<<std::endl;
    }

    MPI_Finalize();
    return !result;
}