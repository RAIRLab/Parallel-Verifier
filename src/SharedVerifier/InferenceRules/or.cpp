

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool isOrVertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "or";
}

bool verifyOrIntroSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    if (!isOrVertex(pn)) {
        return false;
    }

    // Make sure we have one parent nodes
    if (pn.parents.size() != 1) {
        return false;
    }

    // Make sure the sub-formulas match the parents
    const VertId parentId = *pn.parents.begin();
    const Proof::Node& parent_pn = p.nodeLookup.at(parentId);

    return parent_pn.formula == pn.formula.members[1] || \
           parent_pn.formula == pn.formula.members[2];
}

bool verifyOrIntroSemantics(const Proof& p, const VertId vertex_id,
                            Assumptions& assumptions) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const VertId parentId = *pn.parents.begin();
    assumptions[vertex_id] = assumptions[parentId];
    return true;
}

bool verifyOrElimSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have three parent nodes
    if (pn.parents.size() != 3) {
        return false;
    }

    return true;
}

bool verifyOrElimSemantics(const Proof& p, VertId vertex_id, Assumptions& assumptions) {
    
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    optVertId orParentId = {}, parentId2 = {}, parentId3 = {};
    for (const VertId parentId : pn.parents) {
        const Proof::Node& parentNode = p.nodeLookup.at(parentId);
        if (parentNode.formula == pn.formula) {
            if (!parentId2) {
                parentId2 = parentId;
            } else {
                parentId3 = parentId;
            }
        } else {
            // Parent vertices that don't match us must be OR rooted.
            if (!isOrVertex(parentNode)) {
                return false;
            }
            orParentId = parentId;
        }
    }

    // Syntax check: Two of the parents must match the current formula
    // One of the nodes must be OR rooted
    if (!orParentId.has_value() || \
            !parentId2.has_value() || \
            !parentId3.has_value()) {
        return false;
    }

    const Proof::Node& orParent = p.nodeLookup.at(orParentId.value());

    // Check Assumptions
    bool left_side_check = false;
    VertId parent2AssumptionId;
    bool right_side_check = false;
    VertId parent3AssumptionId;

    for (const VertId a : assumptions[parentId2.value()]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (!left_side_check && aNode.formula == orParent.formula.members[1]) {
            left_side_check = true;
            parent2AssumptionId = a;
            break;
        }
        if (!right_side_check && aNode.formula == orParent.formula.members[2]) {
            right_side_check = true;
            parent2AssumptionId = a;
            break;
        }
    }

    for (const VertId a : assumptions[parentId3.value()]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (!left_side_check && aNode.formula == orParent.formula.members[1]) {
            left_side_check = true;
            parent3AssumptionId = a;
            break;
        }
        if (!right_side_check && aNode.formula == orParent.formula.members[2]) {
            right_side_check = true;
            parent3AssumptionId = a;
            break;
        }
    }


    if(!(left_side_check && right_side_check)){
        return false;
    }
        
    assumptions[vertex_id] = assumptions[orParentId.value()];
    std::unordered_set<VertId> parent2Assumptions = assumptions[parentId2.value()];
    parent2Assumptions.erase(parent2AssumptionId);
    std::unordered_set<VertId> parent3Assumptions = assumptions[parentId3.value()];
    parent3Assumptions.erase(parent3AssumptionId);
    assumptions[vertex_id].insert(parent2Assumptions.begin(), parent2Assumptions.end());
    assumptions[vertex_id].insert(parent3Assumptions.begin(), parent3Assumptions.end());
    return true;
}