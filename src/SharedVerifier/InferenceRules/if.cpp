


#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool isIfVertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "if";
}

bool verifyIfIntroSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    // Make sure we're an if vertex only have one parent node
    if (!isIfVertex(pn) && pn.parents.size() != 1) {
        return false;
    }

    // Make sure parent is the consequent
    const VertId parentId = *pn.parents.begin();
    const Proof::Node& parent_pn = p.nodeLookup.at(parentId);
    if (parent_pn.formula != pn.formula.members[2]) {
        return false;
    }

    return true;
}

bool verifyIfIntroSemantics(const Proof& p,
                            const VertId vertex_id,
                            const Assumptions& assumptions,
                            std::unordered_set<VertId>& aIds) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const VertId parentId = *pn.parents.begin();

    // Make sure the antecedent is in the assumptions of the parent
    optVertId antecedentId = {};
    for (const VertId a : assumptions.at(parentId)) {
        const Proof::Node& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            antecedentId = a;
        }
    }
    if (!antecedentId.has_value()) {
        return false;
    }

    // Update Assumptions
    aIds = assumptions.at(parentId);
    aIds.erase(antecedentId.value());
    return true;
    
}

bool verifyIfElimSyntax(const Proof& p, const VertId vertex_id) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    // Check the the current node matches the consequent of the if vertex
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);
    if (isIfVertex(firstParent) && pn.formula != firstParent.formula.members[2]) {
        return false;
    } else if (isIfVertex(secondParent) && pn.formula != secondParent.formula.members[2]) {
        return false;
    }

    // Check that the other parent node matches the antecedent of the if vertex
    return (isIfVertex(firstParent) && \
            secondParent.formula == firstParent.formula.members[1]) || \
           (isIfVertex(secondParent) && \
            firstParent.formula == secondParent.formula.members[1]);
}

bool verifyIfElimSemantics(const Proof& p,
                           const VertId vertex_id,
                           const Assumptions& assumptions,
                           std::unordered_set<VertId>& aIds){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const std::unordered_set<int>& p2Assumptions = assumptions.at(parents[1]);

    // Update Assumptions
    aIds = assumptions.at(parents[0]);
    aIds.insert(p2Assumptions.begin(), p2Assumptions.end());
    return true;
}
