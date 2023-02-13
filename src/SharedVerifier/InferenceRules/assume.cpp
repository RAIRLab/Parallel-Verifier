

#include"../../ProofIO/Proof.hpp"
#include"../SharedVerifier.hpp"

bool verifyAssumptionSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    return pn.parents.size() == 0;
}

bool verifyAssumptionSemantics(const Proof& p, VertId vertex_id, Assumptions& assumptions) {
    assumptions[vertex_id] = {vertex_id};
    return true;
}