
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"

bool OMPVerifier::OMPVerify(const Proof& p){
    const auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);


    Assumptions assumptions;
    for(size_t j = 0; j < layerMap.size(); j++){
        std::vector<VertId> layer(layerMap[j].begin(), layerMap[j].end());

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        bool result = true;
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
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