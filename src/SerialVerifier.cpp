
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include "SharedVerifier/SharedVerifier.hpp"

std::tuple<std::unordered_map<VertId, size_t>, size_t> computerLayerMap(const Proof& p) {
    // VertexId -> Layer Number
    std::unordered_map<VertId, size_t> layerMap;

    // Iterate through all the nodes starting
    // from the assumptions down to the leaves
    std::queue<std::tuple<VertId, size_t>> nodes;

    // Assumptions are at layer 0
    for (VertId vertexId : p.assumptions) {
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

        // Transcend downward: Add children to nodes
        const auto n = p.nodeLookup.at(vertexId);
        for (const VertId &childId : n.children) {
            nodes.push(std::make_tuple(childId, layer + 1));
        }
    }

    return std::make_tuple(layerMap, maxLayerNum);
}

std::vector<std::vector<VertId>> computeLayers(const Proof& p) {
    const auto [layerNum, maxLayerNum] = computerLayerMap(p);

    // Construct layers from layerNum map
    std::vector<std::vector<VertId>> layers(maxLayerNum + 1, std::vector<VertId>());
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
            if (!SharedVerifier::verifyVertex(p, n, assumptions)) {
                return false;
            }
            // Assumptions are updated within verifyVertex function
        }
    }

    return true;
}

int main(int argc, char** argv) {
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof proof(proofFilePath);

    SharedVerifier::startClock();
    bool result = verifySimple(proof);
    SharedVerifier::endClock();

    std::cout << result << std::endl;
    return !result;
}
