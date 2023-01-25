
#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool is_and_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "and";
}

bool verifyAndIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_and_vertex(pn)) {
        return false;
    }

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    // Make sure the subformulas match the parents
    const auto [parent1Id, parent2Id] = [&pn, &p]() {
        optVertId parent1Id = {}, parent2Id = {};
        for (vertId parent_id : pn.parents) {
            const ProofNode& parent_pn = p.nodeLookup.at(parent_id);
            if (!parent1Id && parent_pn.formula == pn.formula.members[1]) {
                parent1Id = parent_id;
            } else if (!parent2Id && parent_pn.formula == pn.formula.members[2]) {
                parent2Id = parent_id;
            }
        }
        return std::make_tuple(parent1Id, parent2Id);
    }();


    const bool result = parent1Id && parent2Id;

    // Update assumptions with union
    if (result) {
        assumptions[vertex_id] = assumptions[parent1Id.value()];
        assumptions[vertex_id].insert(assumptions[parent2Id.value()].begin(), assumptions[parent2Id.value()].end());
    }

    return result;
}

bool verifyAndElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    // Grab parent node
    const vertId parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    if (!is_and_vertex(parent_pn)) {
        return false;
    }

    const bool result = \
        // Check if the formula matches the left parent subformula
        pn.formula == parent_pn.formula.members[1] || \
        // Check if the formula matches the right parent subformula
        pn.formula == parent_pn.formula.members[2];

    // Update assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result;
}
