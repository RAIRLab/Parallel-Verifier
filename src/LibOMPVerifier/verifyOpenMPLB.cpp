
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"
#include<omp.h>

bool OMPVerifier::OMPVerifyLB(const Proof& p){
    const auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);

    // std::vector<VertId> nodes(p.nodeLookup.size());
    // for (const auto &[id, node] : p.nodeLookup) {
    //     nodes.push_back(id);
    // }

    std::vector<std::vector<VertId>> layers;
    for(const std::unordered_set<VertId>& layer : layerMap){
        layers.push_back(std::vector<VertId>(layer.begin(), layer.end()));
    }

    Assumptions assumptions;
    for(size_t j = 0; j < layerMap.size(); j++){
        const std::vector<VertId>& layer = layers[j];

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        bool result = true;
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
        #pragma omp parallel reduction(&&:result)
        {
            int thread_id = omp_get_thread_num();
            int num_threads = omp_get_num_threads();

            size_t elementsPerThread = layer.size() / num_threads;
            size_t start = thread_id * elementsPerThread;
            size_t end = (thread_id + 1) * elementsPerThread;

            /*
                TODO: Don't repeat syntax checks
                - How do I know when a vertex has already been syntax checked?
            */
            for (size_t i = start; i < end; ++i) {
                // TODO: Account for verifyVertexSemantic
                result = SharedVerifier::verifyVertex(p, layer[i], assumptions, resultAssumptions[i]);
            }

            size_t leftOvers = layer.size() % num_threads;
            if (thread_id < leftOvers) {
                // TODO: Account for verifyVertexSemantic
                result = SharedVerifier::verifyVertex(p, layer[end + thread_id], assumptions, resultAssumptions[end + thread_id]);

            } else {
                // Verify only the syntax
                result = SharedVerifier::verifyVertexSyntax(p, layers[j + 1][thread_id]);
            }

        }

        // // Attempt to let OpenMP figure it out, doesn't work sadly
        // #pragma omp parallel 
        // {
        //     #pragma omp for reduction(&&:result)
        //     for(size_t i = 0; i < layer.size(); i++){
        //         result = SharedVerifier::verifyVertexSemantics(p, layer[i], assumptions, resultAssumptions[i]);
        //     }
        //     #pragma omp for reduction(&&:syntaxResult)
        //     for(size_t i = 0; i < nodes.size(); i++){
        //         syntaxResult = SharedVerifier::verifyVertexSyntax(p, nodes[i]);
        //     }
        // }
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
        // TODO: Check that this works as intended
        for(size_t i = 0; i < layer.size(); i++){
            assumptions[layer[i]] = std::move(resultAssumptions[i]);
        }
    }
    return true;
}  