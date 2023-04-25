
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include<list>
#include "../Proof/Proof.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))

//Maps a vertex to the set of assumptions it depends on semantically
//this is NOT all assumptions that are its parents as many rules
//discard assumptions
using Assumptions = std::unordered_map<VertId, std::unordered_set<VertId>>;

//DepthMaps map each node in the proof graph to a layer number
//representing the maximum distance to an assumption
using DepthMap = std::unordered_map<VertId, size_t>;

//LayerMaps store sequential sets of vertices
//index 0 will store nodes on layer 0, index 1 will store
//nodes on layer 1, etc.
using LayerMap = std::vector<std::unordered_set<VertId>>;

using FastLayerMap = std::vector<std::vector<VertId>>;
using FastDepthMap = std::vector<size_t>;
using NodeVec = std::vector<VertId>;

namespace SharedVerifier{

    const char* init(int argc, char** argv);
    
    void startClock();
    std::pair<double, uint64_t> endClock();
    void endClockPrint();
    double endClockSeconds();
    uint64_t endClockCycles();

    // Debug
    std::string assumptionsToString(Assumptions assumptions);

    // Layering
    std::pair<LayerMap, DepthMap> getLayerMap(const Proof& p);
    FastLayerMap getLayerMapFast(const Proof& p);

    //Inference Rule Verification and helpers
    bool verifyVertex(const Proof& p, const VertId vertexId, const Assumptions& assumptions, std::unordered_set<VertId>& aIds);
    bool verifyVertexSyntax(const Proof& p, const VertId vertexId);
    bool verifyVertexSemantics(const Proof& p, const VertId vertexId, const Assumptions& assumptions, std::unordered_set<VertId>& aIds);
}








