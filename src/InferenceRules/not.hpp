
#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool is_not_vertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 2 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "not";
}

bool verifyNotIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    if (!is_not_vertex(pn)) {
        return false;
    }

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    optVertId formula_id = {};
    // Make sure the current formula is a negation of an assumption
    for (const vertId a : assumptions[parents[0]]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            formula_id = a;
            break;
        }
    }
    if (!formula_id) {
        for (const vertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_id) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id.value());
        return true;
    }

    return false;
}

bool verifyNotElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    optVertId formula_id = {};
    // Make sure the current formula is a positive of an assumption
    for (const vertId a : assumptions[parents[0]]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
            formula_id = a;
            break;
        }
    }
    if (!formula_id) {
        for (const vertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_id) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id.value());
        return true;
    }

    return false;
}