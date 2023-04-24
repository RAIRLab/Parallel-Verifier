

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

bool verifyAssumptionSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    return pn.parents.size() == 0;
}

std::pair<bool, Assumptions> verifyAssumptionSemantics(const Proof& p, const VertId vertex_id, const Assumptions& assumptions) {
    Assumptions newAssumption = {
        {vertex_id, {vertex_id}}
    };
    return std::make_pair(true, newAssumption);
}
