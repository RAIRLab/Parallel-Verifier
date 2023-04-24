
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
            assumptions[n] = std::unordered_set<VertId>();
            bool success = SharedVerifier::verifyVertex(p, n, assumptions, assumptions[n]);
            if (!success) {
                return false;
            }
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
