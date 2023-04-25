
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"

using FastLayerMap = std::vector<std::vector<VertId>>;
using FastDepthMap = std::vector<size_t>;
using NodeVec = std::vector<VertId>;


bool OMPVerifier::OMPVerifyOriginal(const Proof& p){

    // Compute Layers in Serial
    // const auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);
    FastLayerMap layerMap = SharedVerifier::getLayerMapFast(p);

    Assumptions assumptions;

    for(size_t j = 0; j < layerMap.size(); j++){
        std::vector<VertId> layer = layerMap[j];

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        // Thread-safe data structures
        bool result = true;
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
        
        // Parallel Reduction
        #pragma omp parallel for reduction (&&:result)
        for(size_t i = 0; i < layer.size(); i++){
            result = SharedVerifier::verifyVertex(p, layer[i], assumptions, resultAssumptions[i]);
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