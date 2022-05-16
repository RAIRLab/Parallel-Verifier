

#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"


inline bool is_if_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "if";
}

bool verifyIfIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_if_vertex(pn)) {
        return false;
    }

    // Make sure we only have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    const vertId parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    // Make sure parent is the consequent
    if (parent_pn.formula != pn.formula.members[2]) {
        return false;
    }

    // Make sure the antecedant is in the assumptions of the parent
    bool antecedant_matched = false;
    vertId antecedantId = 0;
    for (const vertId a : assumptions[parentId]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            antecedant_matched = true;
            antecedantId = a;
        }
    }

    // Update Assumptions
    if (antecedant_matched) {
        assumptions[vertex_id] = assumptions[parentId];
        assumptions[vertex_id].erase(antecedantId);
    }

    return antecedant_matched;
}

bool verifyIfElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool result = \
        // secondParent is the antecedant of firstParent
        (is_if_vertex(firstParent) && secondParent.formula == firstParent.formula.members[1]) || \
        // firstParent is the antecedant of secondParent
        (is_if_vertex(secondParent) && firstParent.formula == secondParent.formula.members[1]);

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    }

    return result;
}

