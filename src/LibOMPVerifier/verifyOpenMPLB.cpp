
#include "../SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier.hpp"
#include<omp.h>

bool OMPVerifier::OMPVerifyLB(const Proof& p){

    // Compute Layers
    const auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);

    // Change from vector<unordered_set<X>> to vector<vector<X>>
    std::vector<std::vector<VertId>> layers;
    layers.reserve(layerMap.size());
    for(const std::unordered_set<VertId>& layer : layerMap){
        layers.push_back(std::vector<VertId>(layer.begin(), layer.end()));
    }

    // // Related to trying to get OpenMP to figure it out
    // std::vector<VertId> nodes(p.nodeLookup.size());
    // for (const auto &[id, node] : p.nodeLookup) {
    //     nodes.push_back(id);
    // }

    // Keep track of nodes that were syntax checked
    std::unordered_map<VertId, bool> syntaxChecked(p.nodeLookup.size());
    for (const auto &[vertId, node] : p.nodeLookup) {
        syntaxChecked[vertId] = false;
    }

    Assumptions assumptions;

    for(size_t j = 0; j < layerMap.size(); j++){
        const std::vector<VertId>& layer = layers[j];

        std::optional<std::vector<VertId>> nextLayer;
        if (j < layerMap.size() - 1) {
            nextLayer = layers[j + 1];
        }

#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        bool result = true;
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
        size_t nextLayerSize = (nextLayer.has_value())? nextLayer.value().size() : 0;
        std::vector<VertId> resultSyntaxCheck(layer.size() + nextLayerSize);
        #pragma omp parallel reduction(&&:result)
        {
            size_t thread_id = omp_get_thread_num();
            size_t num_threads = omp_get_num_threads();

            size_t elementsPerThread = layer.size() / num_threads;
            size_t start = thread_id * elementsPerThread;
            size_t end = (thread_id + 1) * elementsPerThread;

            for (size_t i = start; i < end; ++i) {
                if (syntaxChecked.at(layer[i])) {
                    // If we already verified the syntax, only check the semantics
                    result = SharedVerifier::verifyVertexSemantics(p, layer[i], assumptions, resultAssumptions[i]);
                } else {
                    // Otherwise check both syntax and semantics
                    // std::cout << "HUMANS 1\n"; // TODO: Why doesn't this trigger?
                    result = SharedVerifier::verifyVertex(p, layer[i], assumptions, resultAssumptions[i]);
                    resultSyntaxCheck[i] = true;
                }
            }

            size_t leftOvers = layer.size() % num_threads;
            if (thread_id < leftOvers) {
                // Each thread will verify one of the nodes left
                // on this layer.
                result = SharedVerifier::verifyVertex(p, layer[end + thread_id], assumptions, resultAssumptions[end + thread_id]);
                resultSyntaxCheck[end + thread_id] = true;

            } else if (nextLayer.has_value()) {
                // These threads will look ahead to the next layer and
                // verify a syntax.
                size_t newIndex = (thread_id - leftOvers);
                if (newIndex < nextLayer.value().size()) {
                    /*
                    Current Layer Size: 2 Next Layer Size: 1 newIndex: 0 vertId: 3
                    Current Layer Size: 1 Next Layer Size: 1 newIndex: 0 vertId: 21941
                    terminate called after throwing an instance of 'std::out_of_range'
                      what():  _Map_base::at
                    Aborted (core dumped)
                    Note: the vertId is super high for some reason
                    */
                    // VertId nodeId = (nextLayer.value())[thread_id];
                    // std::string toPrint = std::string("Current Layer Size: ") + std::to_string(layer.size()); 
                    // toPrint += std::string(" Next Layer Size: ") + std::to_string(nextLayer.value().size());
                    // toPrint += " newIndex: " + std::to_string(newIndex);
                    // toPrint += " vertId: " + std::to_string(nodeId) + std::string("\n");
                    // std::cout << toPrint;
                    // result = SharedVerifier::verifyVertexSyntax(p, nodeId);
                    // resultSyntaxCheck[layer.size() + newIndex] = true;
                }

            }

        } // End parallel block
        // std::cout << std::endl;

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

        // Update syntax checked
        // for (size_t i = 0; i < layer.size(); i++) {
        //     syntaxChecked[layer[i]] = resultSyntaxCheck[i];
        // }
        // if (nextLayerExists) {
        //     for (size_t i = 0; i < nextLayerSize; i++) {
        //         syntaxChecked[nextLayer[i]] = resultSyntaxCheck[layer.size() + i];
        //     }
        // }


    }
    return true;
}  