
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"


//Recursive helper for getLayerAndDepthMapsSerial
//Returns the depth of a node id, in a proof p, using and modifying
//a global depth map for the proof.
size_t getNodeDepthBF(const Proof& p, VertId id, DepthMap& depthMap){
    //Base case 1, we already know the depth of the node
    DepthMap::const_iterator depthMapIter = depthMap.find(id);
    if(depthMapIter != depthMap.end()){
        return depthMapIter->second;
    }
    //Base case 2, we're an assumption
    std::unordered_set<VertId> parents = p.nodeLookup.at(id).parents;
    if(parents.size() == 0){
        depthMap[id] = 0;
        return 0;
    }

    //Recursive case
    std::unordered_set<int>::const_iterator itr = parents.begin();
    size_t maxDepth = getNodeDepthBF(p, *itr, depthMap);
    itr++;
    for(;itr != parents.end(); itr++){
        size_t curDepth = getNodeDepthBF(p, *itr, depthMap);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }
    depthMap[id] = maxDepth + 1;
    return maxDepth + 1;
}

using FastLayerMap = std::vector<std::vector<VertId>>;
using NodeVec = std::vector<VertId>;
//O(n) serial construction of the depth and layer maps for a proof.
//n is the number of nodes in the proof
std::pair<FastLayerMap, NodeVec> getLayerMapBF(const Proof& p){
    //Depthmap construction is O(n) via dynamic programming
    DepthMap depthMap;
    NodeVec allNodes;
    allNodes.reserve(p.nodeLookup.size());
    size_t maxDepth = 0;
    for(const auto& [id, node] : p.nodeLookup){
        size_t curDepth = getNodeDepthBF(p, id, depthMap);
        allNodes.push_back(id);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }

    //LayerMap construction O(n)
    //std::list fill constructor
    FastLayerMap layerVector(maxDepth + 1, std::vector<VertId>());
    for(const auto& [id, depth] : depthMap){
        layerVector[depth].push_back(id);
    }

    return std::make_pair(layerVector, allNodes);
}

bool OMPVerifier::OMPVerifyBF(const Proof& p){

    // Compute Layers in Serial
    auto [layerMap, allNodes] = getLayerMapBF(p);

    bool result = true;
    #pragma omp parallel for reduction (&&:result)
    for(int i = 0; i < allNodes.size(); i++){
        result = SharedVerifier::verifyVertexSyntax(p, allNodes[i]);
    }
    if (!result) {
        return false;
    }

    Assumptions assumptions;
    for(size_t j = 0; j < layerMap.size(); j++){
        std::vector<VertId> layer = layerMap[j];

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        // Thread-safe data structures
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
        
        // Parallel Reduction
        #pragma omp parallel for reduction (&&:result)
        for(size_t i = 0; i < layer.size(); i++){
            result = SharedVerifier::verifyVertexSemantics(p, layer[i], assumptions, resultAssumptions[i]);
        }

        if (!result) {
            return false;
        }

#ifdef PRINT_DEBUG
        std::cout<<"Result Assumptions: ";
        for(size_t i = 0; i < layer.size(); i++){
            std::cout<<layer[i]<<":(";
            for(VertId assumpt : resultAssumptions[i]){
                std::cout<<assumpt<<",";
            }
            std::cout<<"\b),";
        }
        std::cout<<"\b"<<std::endl;
#endif
        // Update assumptions for each node we just verified
        for(size_t i = 0; i < layer.size(); i++){
            assumptions[layer[i]] = std::move(resultAssumptions[i]);
        } 
    }
    return true;
}  