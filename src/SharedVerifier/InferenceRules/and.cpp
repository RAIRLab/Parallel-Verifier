

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool isAndVertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "and";
}

bool verifyAndIntroSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    //Current vertex syntax correct
    if (!isAndVertex(pn) && pn.parents.size() != 2)
        return false;

    //Parent Vertex Syntax Correct
    // Make sure the sub-formulas match the parents
    optVertId parent1Id = {}, parent2Id = {};
    for (VertId parent_id : pn.parents) {
        const Proof::Node& parent_pn = p.nodeLookup.at(parent_id);
        if (!parent1Id && parent_pn.formula == pn.formula.members[1]) {
            parent1Id = parent_id;
        } else if (!parent2Id && parent_pn.formula == pn.formula.members[2]) {
            parent2Id = parent_id;
        }
    }

    return parent1Id.has_value() && parent2Id.has_value();
}

//Precondition: verifyAndIntroSyntax has been called already
//Just updates the assumptions since AndIntro is non-context changing
bool verifyAndIntroSemantics(const Proof& p,
                             const VertId vertex_id, 
                             const Assumptions& assumptions,
                             std::unordered_set<VertId>& aIds){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    VertId parent1Id = parents[0];
    VertId parent2Id = parents[1];
    const std::unordered_set<int>& p2Assumptions = assumptions.at(parent2Id);

    aIds = assumptions.at(parent1Id);
    aIds.insert(p2Assumptions.begin(), p2Assumptions.end());
    
    return true;
}

bool verifyAndElimSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have one parent node
    if (pn.parents.size() != 1)
        return false;

    // Grab parent node
    const VertId parentId = *pn.parents.begin();
    const Proof::Node& parent_pn = p.nodeLookup.at(parentId);
    if (!isAndVertex(parent_pn)) {
        return false;
    }
    // Check if the formula matches either the left or right parent sub-formula
    
    return pn.formula == parent_pn.formula.members[1] || \
           pn.formula == parent_pn.formula.members[2];
}

bool verifyAndElimSemantics(const Proof& p,
                            const VertId vertex_id,
                            const Assumptions& assumptions,
                            std::unordered_set<VertId>& aIds) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const VertId parentId = *pn.parents.begin();
    // Update Assumption Ids
    aIds = assumptions.at(parentId);
    return true;
}
