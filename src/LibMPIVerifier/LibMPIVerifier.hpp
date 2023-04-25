
#pragma once

#include<utility>
#include<cmath>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<list>
#include<numeric>

#include "../Proof/Proof.hpp"
#include "../ProofIO/ProofIO.hpp"
#include "../SharedVerifier/SharedVerifier.hpp"

//LayerMappers are functions that compute a layerMap from a proof.
using LayerMapper = std::pair<LayerMap,DepthMap>(*)(const Proof&);

namespace ParallelVerifier{
    void updateAssumptions(Assumptions& assumptions,
                           const std::list<VertId>& checked);
    
    //Provided LayerMappers
    std::pair<LayerMap,DepthMap> getLayerMapMPI(const Proof& proof);

    //The classic verifier from sp22 class
    bool verifyAlpha(const Proof& proof);

    //Provided MPI Verifiers with different optimizations  
    bool verifyParallelOriginal(const Proof& proof, LayerMapper mapper);
    bool verifyParallelNoOpt(const Proof& proof, LayerMapper mapper);
    bool verifyParallelLoadBalance(const Proof& proof, LayerMapper mapper);
}