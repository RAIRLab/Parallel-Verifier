
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"

bool OMPVerifier::OMPVerify(const Proof& p){
    auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);
    
    std::vector<std::vector<VertId>> layers;
    for(const std::unordered_set<VertId>& layer : layerMap){
        layers.push_back(std::vector<VertId>(layer.begin(), layer.end()));
    }

    Assumptions assumptions;
    for(size_t j = 0; j < layers.size(); j++){
        const std::vector<VertId>& layer = layers[j];

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        std::vector<uint8_t> results(layer.size());
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size());
        //Need private per thread copy of assumptions since we can't insert in parallel 
        #pragma omp parallel for firstprivate(assumptions)
        for(size_t i = 0; i < layer.size(); i++){
            VertId node = layer[i];
            results[i] = SharedVerifier::verifyVertex(p, node, assumptions);
            resultAssumptions[i] = assumptions[node];
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
        //update assumptions and check if we've failed in serial
        bool failed = false; 
        for(size_t i = 0; i < layer.size(); i++){
            VertId node = layer[i];
            //This update is necessary because the "resultAssumptions[i] = assumptions[node];"
            //inside the "omp parallel for" is a local thread copy.
            assumptions[node] = resultAssumptions[i];
            failed |= !results[i];
        }

        if(failed){
            return false;
        }
    }
    return true;
}  