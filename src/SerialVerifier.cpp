
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include"SharedVerifier.hpp"
#include"timing.hpp"

bool verifySimple(Proof& p) {

    // VertexId -> Layer Number
    std::unordered_map<vertId, int> layerNum;
    
    // Iterate through all the nodes starting
    // from the assumptions down to the leaves
    std::queue<std::tuple<vertId, int>> nodes;
    // Assumptions are at layer 0
    for (vertId vertexId: p.assumptions) {
        nodes.push(std::make_tuple(vertexId, 0))
    }
    int maxLayerNum = 0;
    while (!nodes.empty()) {
        // Grab next vertexId
        const vertId vertexId, const int layer;
        std::tie(vertexId, layer) = nodes.front();
        nodes.pop();

        // Replace any lower value layer number with the current layer
        // number
        layerNum[vertexId] = layer;

        // Keep track of the max layer number
        if (layer > maxLayerNum) {
            maxLayerNum = layer;
        }

        // Trascend downward: Add children to nodes
        for (const vertId childId: n.children) {
            nodes.push(std::make_tuple(childId, layer + 1))
        }
    }

    // Construct layers from layerNum map
    std::vector<std::queue<vertId>> layers;
    for (int i = 0; i < maxLayerNum; i++) {
        layers[i] = std::queue<vertId>;
    }
    for (const auto & [ vertexId, layer ] : layerNum) {
        layers[layer].push(vertexId);
    }

    Assumptions assumptions;

    for (const std::queue<vertId> currentLayer: layers) {
        for (const vertId n: currentLayer) {
            if (!verifyVertex(p, vid, assumptions)) {
                return false;
            }
            // Assumptions are updated within verifyVertex function
        }
    }

    return true;
}

int main(int argc, char** argv){
    const char* proofFilePath = VerifierInit(argc, argv);
    std::ifstream fileStream(proofFilePath);
    std::string fileConents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    Proof proof(fileConents);

    startClock();
    bool result = verifySimple(proof);
    endClock();

    std::cout<<result<<std::endl;
    return 0;
}