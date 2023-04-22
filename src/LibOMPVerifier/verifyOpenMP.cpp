
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"

bool OMPVerifier::OMPVerify(const Proof& p){
    auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);
    
    std::vector<std::vector<VertId>> layers;
    for(const std::unordered_set<VertId>& layer : layerMap){
        layers.push_back(std::vector<VertId>(layer.begin(), layer.end()));
    }

    Assumptions assumptions;
    for(const std::vector<VertId>& layer : layers){
        std::vector<uint8_t> results(layer.size(), 1);
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size());
        //Need private per thread copy of assumptions since we can't insert in parallel 
        #pragma omp parallel for private(assumptions)
        for(size_t i = 0; i < layer.size(); i++){
            VertId node = layer[i];
            results[i] = SharedVerifier::verifyVertex(p, node, assumptions);
            resultAssumptions[i] = assumptions[node];
        }

        //update assumptions and check if we've failed in serial
        bool failed = false; 
        for(size_t i = 0; i < layer.size(); i++){
            VertId node = layer[i];
            assumptions[node] = resultAssumptions[i];
            failed |= !results[i];
        }

        if(failed){
            return false;
        }
    }
    return true;
}  