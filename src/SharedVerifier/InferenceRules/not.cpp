

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool isNotVertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 2 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "not";
}

bool verifyNotIntroSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    if (!isNotVertex(pn) && pn.parents.size() != 2) {
        return false;
    }

    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);
    const bool syntax_result = \
        // Check to see if the first parent is the negation of the second
        (isNotVertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (isNotVertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    return syntax_result;
}

bool verifyNotIntroSemantics(const Proof& p, const VertId vertex_id, Assumptions& assumptions) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());

    optVertId formula_id = {};
    // Make sure the current formula is a negation of an assumption
    for (const VertId a : assumptions[parents[0]]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            formula_id = a;
            break;
        }
    }
    if (!formula_id.has_value()) {
        for (const VertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                formula_id = a;
                break;
            }
        }
    }
    if (!formula_id.has_value()){
        return false;
    }

    // Update Assumptions
    assumptions[vertex_id] = assumptions[parents[0]];
    assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    assumptions[vertex_id].erase(formula_id.value());
    return true;
}

bool verifyNotElimSyntax(const Proof& p, VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the negation of the second
        (isNotVertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (isNotVertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    return true;
}

bool verifyNotElimSemantics(const Proof& p, VertId vertex_id, Assumptions& assumptions) {
    
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    optVertId formula_id = {};
    // Make sure the current formula is a positive of an assumption
    for (const VertId a : assumptions[parents[0]]) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (isNotVertex(aNode) && aNode.formula.members[1] == pn.formula) {
            formula_id = a;
            break;
        }
    }
    if (!formula_id.has_value()) {
        for (const VertId a : assumptions[parents[1]]) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (isNotVertex(aNode) && aNode.formula.members[1] == pn.formula) {
                formula_id = a;
                break;
            }
        }
    }
    if (!formula_id.has_value()) {
        return false;
    }

    // Update Assumptions
    assumptions[vertex_id] = assumptions[parents[0]];
    assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    assumptions[vertex_id].erase(formula_id.value());
    return true;
}