

#include"../../Proof/Proof.hpp"
#include"../SharedVerifier.hpp"

inline bool isIffVertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "iff";
}

bool verifyIffIntroSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    // Make sure we only have two parent nodes
    if (!isIffVertex(pn) && pn.parents.size() != 2) {
        return false;
    }
    return true;
}

bool verifyIffIntroSemantics(const Proof& p,
                             const VertId vertex_id,
                             const Assumptions& assumptions,
                             std::unordered_set<VertId>& aIds) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);

    // Check forward direction
    bool forward_check = false;
    VertId forward_id;
    if (firstParent.formula == pn.formula.members[2]) {
        // Make sure the antecedent is in the assumptions of the parent
        for (const VertId a : assumptions.at(parents[0])) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
                break;
            }
        }
    } else if (secondParent.formula == pn.formula.members[2]) {
        // Make sure the antecedent is in the assumptions of the parent
        for (const VertId a : assumptions.at(parents[1])) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
                break;
            }
        }
    }
    if(!forward_check){
        return false;
    }

    // Check backward direction
    bool backward_check = false;
    VertId backward_id;
    if (firstParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const VertId a : assumptions.at(parents[0])) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[2]) {
                backward_check = true;
                backward_id = a;
                break;
            }
        }
    } else if (secondParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const VertId a : assumptions.at(parents[1])) {
            const Proof::Node& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[2]) {
                backward_check = true;
                backward_id = a;
                break;
            }
        }
    }
    if(!backward_check){
        return false;
    }

    // Update Assumptions
    aIds = assumptions.at(parents[0]);
    aIds.insert(assumptions.at(parents[1]).begin(), assumptions.at(parents[1]).end());
    aIds.erase(forward_id);
    aIds.erase(backward_id);
    return true;
}

bool verifyIffElimSyntax(const Proof& p, const VertId vertex_id){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    // Check that the current node matches one of the parents
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());
    const Proof::Node& firstParent = p.nodeLookup.at(parents[0]);
    const Proof::Node& secondParent = p.nodeLookup.at(parents[1]);
    int parentMatchInd = -1;
    if (isIffVertex(firstParent) && \
              (pn.formula == firstParent.formula.members[1] || \
              pn.formula == firstParent.formula.members[2])){
        parentMatchInd = 0;
    } else if (isIffVertex(secondParent) && \
              (pn.formula == secondParent.formula.members[1] || \
              pn.formula == secondParent.formula.members[2])) {
        parentMatchInd = 1;
    } else {
        return false;
    }

    // Check that the parents match each other
    if (parentMatchInd == 1) {
        // First parent is either the antecedent
        // or consequent of the second parent.
        bool syntax_check1 = firstParent.formula == secondParent.formula.members[1] || \
            firstParent.formula == secondParent.formula.members[2];
        if (!syntax_check1) {
            return false;
        }
    } else {
        // Second parent is either the antecedent
        // or consequent of the first parent
        bool syntax_check1 = secondParent.formula  == firstParent.formula.members[1] || \
            secondParent.formula  == firstParent.formula.members[2];
        if (!syntax_check1) {
            return false;
        }
    }

    return true;
}

bool verifyIffElimSemantics(const Proof& p,
                            const VertId vertex_id,
                            const Assumptions& assumptions,
                            std::unordered_set<VertId>& aIds) {
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    const std::vector<VertId> parents(pn.parents.begin(), pn.parents.end());

    // Update Assumptions
    aIds = assumptions.at(parents[0]);
    aIds.insert(assumptions.at(parents[1]).begin(), assumptions.at(parents[1]).end());

    return true;
}