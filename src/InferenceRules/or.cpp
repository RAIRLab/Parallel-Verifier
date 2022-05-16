

#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool is_or_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "or";
}

bool verifyOrIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_or_vertex(pn)) {
        return false;
    }

    // Make sure we have one parent nodes
    if (pn.parents.size() != 1) {
        return false;
    }

    // Make sure the subformulas match the parents
    const vertId parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    const bool result = parent_pn.formula == pn.formula.members[1] || parent_pn.formula == pn.formula.members[2];

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result;
}

bool verifyOrElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have three parent nodes
    if (pn.parents.size() != 3) {
        return false;
    }

    vertId orParentId;
    vertId parentId2;
    vertId parentId3;
    bool orParentFound = false;
    bool parentId2Found = false;
    bool parentId3Found = false;
    for (const vertId parentId : pn.parents) {
        const ProofNode& parentNode = p.nodeLookup.at(parentId);
        if (parentNode.formula == pn.formula) {
            if (!parentId2Found) {
                parentId2 = parentId;
                parentId2Found = true;
            } else {
                parentId3 = parentId;
                parentId3Found = true;
            }
        } else {
            // Parent vertices that don't match us must be OR rooted.
            if (!is_or_vertex(parentNode)) {
                return false;
            }
            orParentFound = true;
            orParentId = parentId;
        }
    }

    // Syntax check: Two of the parents must match the current formula
    // One of the nodes must be OR rooted
    if (!parentId2Found || !parentId3Found || !orParentFound) {
        return false;
    }

    const ProofNode& orParent = p.nodeLookup.at(orParentId);
    const ProofNode& parentNode2 = p.nodeLookup.at(parentId2);
    const ProofNode& parentNode3 = p.nodeLookup.at(parentId3);

    // Check Assumptions
    bool left_side_check = false;
    vertId parent2AssumptionId;
    bool right_side_check = false;
    vertId parent3AssumptionId;

    for (const vertId a : assumptions[parentId2]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
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

    for (const vertId a : assumptions[parentId3]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
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


    const bool result = left_side_check && right_side_check;
    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[orParentId];

        std::unordered_set<vertId> parent2Assumptions = assumptions[parentId2];
        parent2Assumptions.erase(parent2AssumptionId);
        std::unordered_set<vertId> parent3Assumptions = assumptions[parentId3];
        parent3Assumptions.erase(parent3AssumptionId);

        assumptions[vertex_id].insert(parent2Assumptions.begin(), parent2Assumptions.end());
        assumptions[vertex_id].insert(parent3Assumptions.begin(), parent3Assumptions.end());
    }

    return result;
}