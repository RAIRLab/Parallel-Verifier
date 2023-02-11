
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../ProofIO/Proof.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))

namespace SharedVerifier{

    using Assumptions = std::unordered_map<vertId, std::unordered_set<vertId>>;

    const char* init(int argc, char** argv);
    
    void startClock();
    void endClock();

    //Inference Rule Verification and helpers
    bool verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions);
    bool verifyVertexSyntax(const Proof& p, vertId vertex_id);
    bool verifyVertexSemantics(const Proof& p, vertId vertex_id, Assumptions& assumptions);
}








