
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

//DepthMaps map each node in the proof graph to a layer number
//representing the maximum distance to an assumption
using DepthMap = std::unordered_map<VertId, size_t>;

//LayerMaps store sequential sets of vertices
//index 0 will store nodes on layer 0, index 1 will store
//nodes on layer 1, etc.
using LayerMap = std::list<std::unordered_set<VertId>>;

//LayerMappers are functions that compute a layerMap from a proof.
using LayerMapper = std::pair<LayerMap,DepthMap>(*)(const Proof&);

namespace ParallelVerifier{
    //Provided LayerMappers
    std::pair<LayerMap,DepthMap> getLayerMapSerial(const Proof& proof);
    std::pair<LayerMap,DepthMap> getLayerMapMPI(const Proof& proof);

    //Provided Verifiers with different optimizations  
    bool verifyParallelNoOpt(const Proof& proof, LayerMapper mapper);
    bool verifyParallelLoadBalance(const Proof& proof, LayerMapper mapper);
    bool verifyParallelSemanticJump(const Proof& proof, LayerMapper mapper);
}