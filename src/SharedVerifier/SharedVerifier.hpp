
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../Proof/Proof.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))

//Maps a vertex to the set of assumptions it depends on semantically
//this is NOT all assumptions that are its parents as many rules
//discard assumptions
using Assumptions = std::unordered_map<VertId, std::unordered_set<VertId>>;

namespace SharedVerifier{

    const char* init(int* argc, char*** argv);
    
    void startClock();
    std::pair<double, uint64_t> endClock();
    void endClockPrint();
    double endClockSeconds();
    uint64_t endClockCycles();

    //Inference Rule Verification and helpers
    bool verifyVertex(const Proof& p, const VertId vertexId, Assumptions& assumptions);
    bool verifyVertexSyntax(const Proof& p, const VertId vertexId);
    bool verifyVertexSemantics(const Proof& p, const VertId vertexId, Assumptions& assumptions);
}








