
#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool is_iff_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "iff";
}

bool verifyIffIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_iff_vertex(pn)) {
        return false;
    }

    // Make sure we only have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    bool forward_check = false;
    vertId forward_id;
    bool backward_check = false;
    vertId backward_id;

    // Check forward direction
    if (!forward_check && firstParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const vertId a : assumptions[parents[0]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
                break;
            }
        }
    } else if (!forward_check && secondParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const vertId a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
                break;
            }
        }
    }

    // Check backward direction
    if (!backward_check && firstParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const vertId a : assumptions[parents[0]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[2]) {
                backward_check = true;
                backward_id = a;
                break;
            }
        }
    } else if (!backward_check && secondParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const vertId a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[2]) {
                backward_check = true;
                backward_id = a;
                break;
            }
        }
    }

    const bool result = forward_check && backward_check;

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(forward_id);
        assumptions[vertex_id].erase(backward_id);
    }

    return result;
}

bool verifyIffElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool result =  \
        // secondParent is either the antecedant or consequent of firstParent
        (is_iff_vertex(firstParent) && \
            (secondParent.formula == firstParent.formula.members[1] || \
            secondParent.formula == firstParent.formula.members[2])
        ) || \
        // firstParent is either the antecedant or consequent of secondParent
        (is_iff_vertex(secondParent) && \
            (firstParent.formula == secondParent.formula.members[1] || \
            firstParent.formula == secondParent.formula.members[2])
        );

    if (result) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    }

    return result;
}