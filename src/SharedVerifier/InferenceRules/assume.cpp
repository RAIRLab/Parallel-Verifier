

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

bool verifyAssumptionSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    return pn.parents.size() == 0;
}

std::pair<bool, std::unordered_set<VertId>> verifyAssumptionSemantics(const Proof& p, const VertId vertex_id, const Assumptions& assumptions) {
    std::unordered_set<VertId> assumptionIds = {vertex_id};
    return std::make_pair(true, assumptionIds);
}
