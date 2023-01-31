

#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"

extern bool is_if_vertex(const ProofNode&);

inline bool is_equals_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "=";
}

bool verifyEqIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_equals_vertex(pn)) {
        return false;
    }

    bool result = pn.formula.members[1] == pn.formula.members[2];

    // No assumptions for equals intro
    if (result) {
        assumptions[vertex_id] = {};
    }

    return result;
}

bool verifyEqElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    //TODO: What is going on here? 
    // REPLACE THIS PART
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