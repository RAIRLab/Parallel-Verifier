
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

    // Keep track of nodes that were syntax checked
    std::unordered_map<VertId, bool> syntaxChecked(p.nodeLookup.size());
    for (const auto &[vertId, node] : p.nodeLookup) {
        syntaxChecked[vertId] = false;
    }

    // Setup thread counts
    size_t num_threads;
    bool envSet = std::sscanf(
        std::getenv("OMP_NUM_THREADS"),
        "%zu", &num_threads
    );
    if (!envSet) {
        throw std::runtime_error("Need to set OMP_NUM_THREADS");
    }

    Assumptions assumptions;

    for(size_t j = 0; j < layerMap.size(); j++){
        const std::vector<VertId>& layer = layers[j];

        const bool nextLayerExists = j < layerMap.size() - 1;
        const size_t nextLayerSize = (nextLayerExists)? layers[j + 1].size() : 0;


#ifdef PRINT_DEBUG
        std::cout<< "Layer " <<j<<" Assumptions "<< \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif

        // Thread safe data structures
        bool result = true;
        std::vector<std::unordered_set<VertId>> resultAssumptions(layer.size(), std::unordered_set<VertId>());
        std::vector<VertId> resultSyntaxCheck(layer.size() + nextLayerSize);

        const size_t leftOvers = layer.size() % num_threads;
        const size_t elementsPerThread = layer.size() / num_threads;
        const size_t lastNameBetter = num_threads * elementsPerThread;

        // std::cout << "Layer Size: " << layer.size() << std::endl;
        // std::cout << "Next Layer Size: " << nextLayerSize << std::endl;
        // std::cout << "Number Threads: " << num_threads << std::endl;

        // std::cout << "Layer: ";
        // for (auto vertId : layer) {
            // std::cout << vertId << ", ";
        // }
        // std::cout << std::endl;


        omp_set_dynamic(0);
        omp_set_num_threads(num_threads);
        #pragma omp parallel reduction(&&:result)
        {
            const size_t thread_id = omp_get_thread_num();
            const size_t start = thread_id * elementsPerThread;
            const size_t end = (thread_id + 1) * elementsPerThread;

            for (size_t i = start; i < end; i++) {
                if (syntaxChecked.at(layer[i])) {
                    // std::string toPrint3 = std::string("Already Syntax Checked: ");
                    // toPrint3 += std::to_string(layer[i]) += std::string("\n");
                    // std::cout << toPrint3;

                    // If we already verified the syntax, only check the semantics
                    result = SharedVerifier::verifyVertexSemantics(p, layer[i], assumptions, resultAssumptions[i]);

                    // std::string toPrint11 = std::string("Verified semantics of: ");
                    // toPrint11 += std::to_string(layer[i]) + std::string("\n");
                    // std::cout << toPrint11; 

                } else {
                    // Otherwise check both syntax and semantics

                    // std::string toPrint9 = std::string("Full Verifying Vertex: ") + std::to_string(layer[i]);
                    // toPrint9 += std::string("\n");
                    // std::cout << toPrint9;
                    
                    result = SharedVerifier::verifyVertex(p, layer[i], assumptions, resultAssumptions[i]);
                    
                    
                    // std::string toPrint8 = std::string("Verified Fully Vertex: ") + std::to_string(layer[i]); 
                    // toPrint8 += std::string(" Setting RSC[") + std::to_string(i);
                    // toPrint8 += std::string("] to true\n");
                    // std::cout << toPrint8;

                    resultSyntaxCheck[i] = true;

                    // std::cout << "H3 Success\n";
                }
            }

            // size_t leftOvers = layer.size() % num_threads;

            // std::string toPrint0 = std::string("Layer Size: ") + std::to_string(layer.size());
            // toPrint0 += std::string(" ElementsPerThread: ") + std::to_string(elementsPerThread);
            // toPrint0 += std::string(" Leftovers: ") + std::to_string(leftOvers);
            // toPrint0 += std::string(" ForwardCheck: ") + std::to_string(num_threads - leftOvers);
            // toPrint0 += std::string("\n");
            // std::cout << toPrint0;

            if (thread_id < leftOvers) {
                // Each thread will verify one of the nodes left
                // on this layer.
                // std::string toPrint12 = std::string("Verifying LeftOver: ");
                // toPrint12 += std::to_string(layer[lastNameBetter + thread_id]) + std::string("\n");
                // std::cout << toPrint12;

                if (syntaxChecked.at(lastNameBetter + thread_id)) {
                    result = SharedVerifier::verifyVertexSemantics(p, layer[lastNameBetter + thread_id], assumptions, resultAssumptions[lastNameBetter + thread_id]);
                } else {
                    result = SharedVerifier::verifyVertex(p, layer[lastNameBetter + thread_id], assumptions, resultAssumptions[lastNameBetter + thread_id]);
                }
                
                // std::string toPrint5 = std::string("Verified Leftover: ") + std::to_string(layer[lastNameBetter + thread_id]); 
                // toPrint5 += std::string(" Setting RSC[") + std::to_string(lastNameBetter + thread_id);
                // toPrint5 += std::string("] to true\n");
                // std::cout << toPrint5;
                
                resultSyntaxCheck[lastNameBetter + thread_id] = true;
                
                // std::cout << "H1 Success\n";
            } else if (nextLayerExists) {
                // These threads will look ahead to the next layer and
                // verify a syntax.
                size_t newIndex = (thread_id - leftOvers);
                if (newIndex < layers[j + 1].size()) {
                    VertId nodeId = layers[j + 1].at(newIndex);

                    // std::string toPrint = std::string("Forward Checking: ");
                    // toPrint += std::to_string(nodeId) + std::string("\n");
                    // std::cout << toPrint;


                    result = SharedVerifier::verifyVertexSyntax(p, nodeId);

                    // std::string toPrint7 = std::string("Verified FC: ") + std::to_string(nodeId);
                    // toPrint7 += std::string(" Setting RSC[") + std::to_string(layer.size() + newIndex);
                    // toPrint7 += std::string("] to true\n");
                    // std::cout << toPrint7;

                    resultSyntaxCheck[layer.size() + newIndex] = true;
                    // std::cout << "H2 Success\n";
                }

            }

        } // End parallel block
        

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
        // std::cout << "Current Layer RSC copy" << std::endl;
        for (size_t i = 0; i < layer.size(); i++) {
            syntaxChecked[layer[i]] = resultSyntaxCheck[i];
        }
        // std::cout << "Finished copy" << std::endl;

        if (nextLayerExists) {
            // std::cout << "currentLayer size: " << layer.size() << std::endl;
            // std::cout << "nextLayer size: " << nextLayer.value().size() << std::endl;
            // std::cout << "RSC size: " << resultSyntaxCheck.size() << std::endl;
            for (size_t i = 0; i < (num_threads - leftOvers) && i < layers[j + 1].size(); i++) {
                // std::cout << "Inner: " << layer.size() + i << std::endl;
                syntaxChecked[layers[j + 1].at(i)] = resultSyntaxCheck[layer.size() + i];
            }
        }

        // std::cout << std::endl; // Space between each layer


    }
    return true;
}  