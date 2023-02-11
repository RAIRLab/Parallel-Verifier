
#pragma once
#include"../../ProofIO/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool is_iff_vertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "iff";
}

bool verifyIffIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    if (!is_iff_vertex(pn)) {
        return false;
    }

    // Make sure we only have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    bool forward_check = false;
    vertId forward_id;
    bool backward_check = false;
    vertId backward_id;

    // Check forward direction
    if (!forward_check && firstParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const vertId a : assumptions[parents[0]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
                break;
            }
        }
    } else if (!forward_check && secondParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const vertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
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
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[2]) {
                backward_check = true;
                backward_id = a;
                break;
            }
        }
    } else if (!backward_check && secondParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const vertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
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
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    int parentMatchInd = -1;

    // Check that the current node matches one of the parents
    if (is_iff_vertex(firstParent) && \
            (pn.formula == firstParent.formula.members[1] || \
            pn.formula == firstParent.formula.members[2])) {
                parentMatchInd = 0;
    } else if (is_iff_vertex(secondParent) && \
            (pn.formula == secondParent.formula.members[1] || \
            pn.formula == secondParent.formula.members[2])) {
                parentMatchInd = 1;
    } else {
        return false;
    }

    // Check that the parents match each other
    if (parentMatchInd == 1) {
        // First parent is either the antecedant
        // or consequent of the second parent.
        bool syntax_check1 = firstParent.formula == secondParent.formula.members[1] || \
            firstParent.formula == secondParent.formula.members[2];
        if (!syntax_check1) {
            return false;
        }
    } else {
        // Second parent is either the antecedant
        // or consequent of the first parent
        bool syntax_check1 = secondParent.formula  == firstParent.formula.members[1] || \
            secondParent.formula  == firstParent.formula.members[2];
        if (!syntax_check1) {
            return false;
        }
    }

    // Update Assumptions
    assumptions[vertex_id] = assumptions[parents[0]];
    assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());

    return true;
}