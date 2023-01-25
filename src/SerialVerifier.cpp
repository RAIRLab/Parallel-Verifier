
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include"SharedVerifier.hpp"
#include"timing.hpp"

std::tuple<std::unordered_map<vertId, size_t>, size_t> computerLayerMap(const Proof& p) {
    // VertexId -> Layer Number
    std::unordered_map<vertId, size_t> layerMap;

    // Iterate through all the nodes starting
    // from the assumptions down to the leaves
    std::queue<std::tuple<vertId, size_t>> nodes;

    // Assumptions are at layer 0
    for (vertId vertexId : p.assumptions) {
        nodes.push(std::make_tuple(vertexId, 0));
    }

    size_t maxLayerNum = 0;
    while (!nodes.empty()) {
        // Grab next vertexId
        const auto [vertexId, layer] = nodes.front();
        nodes.pop();

        // Replace any lower value layer number with the current layer
        // number
        layerMap[vertexId] = layer;

        // Keep track of the max layer number
        maxLayerNum = std::max(maxLayerNum, layer);

        // Trascend downward: Add children to nodes
        const auto n = p.nodeLookup.at(vertexId);
        for (const vertId &childId : n.children) {
            nodes.push(std::make_tuple(childId, layer + 1));
        }
    }

    return std::make_tuple(layerMap, maxLayerNum);
}

std::vector<std::vector<vertId>> computeLayers(const Proof& p) {
    const auto [layerNum, maxLayerNum] = computerLayerMap(p);

    // Construct layers from layerNum map
    std::vector<std::vector<vertId>> layers(maxLayerNum + 1, std::vector<vertId>());
    for (const auto & [ vertexId, layer ] : layerNum) {
        layers.at(layer).push_back(vertexId);
    }

    return layers;
}

bool verifySimple(const Proof& p) {
    Assumptions assumptions;
    const auto layers = computeLayers(p);

    for (const auto &currentLayer : layers) {
        for (const auto &n : currentLayer) {
            if (!verifyVertex(p, n, assumptions)) {
                return false;
            }
            // Assumptions are updated within verifyVertex function
        }
    }

    return true;
}

int main(int argc, char** argv) {
    const char* proofFilePath = VerifierInit(argc, argv);
    std::ifstream fileStream(proofFilePath);
    std::string fileConents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    Proof proof(fileConents);

    startClock();
    bool result = verifySimple(proof);
    endClock();

    std::cout << result << std::endl;
    return 0;
}
