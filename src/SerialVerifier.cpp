
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "SharedVerifier/SharedVerifier.hpp"

bool verifySimple(const Proof& p) {
    Assumptions assumptions;
    const auto [layerMap, depthMap] = SharedVerifier::getLayerMap(p);

    for (const auto &currentLayer : layerMap) {
        for (const auto &n : currentLayer) {
            // Assumptions are updated within verifyVertex function
            auto [success, newAssumptions] = SharedVerifier::verifyVertex(p, n, assumptions);
            if (!success) {
                return false;
            }
            // Update assumptions
            assumptions.merge(newAssumptions);
        }
    }

    return true;
}

int main(int argc, char** argv) {
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof proof(proofFilePath);
    
    SharedVerifier::startClock();
    bool result = verifySimple(proof);
    SharedVerifier::endClockPrint();
    std::cout << ", " << !result << " Result Code" << std::endl;

    return !result;
}
