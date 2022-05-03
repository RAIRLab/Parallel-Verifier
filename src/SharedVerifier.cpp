

#include"SharedVerifier.hpp"

//Marking code ====================================================================================

// Update dest's marking vector to contain source -Brandon
void mark(Markings& markings, vertId source, vertId dest) {
    Markings::iterator it = markings.find(dest);
    if (it == markings.end()) {
        markings[dest] = std::unordered_set<vertId>();
    }
    markings.at(dest).insert(source);
}

//Returns true if the node at vertex_id in proof p has a full list of markings
bool hasCompleteMarkings(const Proof& p, vertId vertex_id, const std::unordered_set<vertId>& markingList) {
    const std::unordered_map<HyperslateJustification, size_t> numMarkingsMap = {
        {Assume, 0}, {AndIntro, 2}, {AndElim, 1},
        {OrIntro, 1}, {OrElim, 3}, {NotIntro, 2},
        {NotElim, 2}, {IfIntro, 1}, {IfElim, 2},
        {IffIntro, 2}, {IffElim, 2}
    };
    size_t markingsNeeded = numMarkingsMap.at(p.nodeLookup.at(vertex_id).justification);
    return markingList.size() == markingsNeeded;
}

//Infrence Rule Verification Helpers ==============================================================

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
    bool match_parent1 = false;
    bool match_parent2 = false;
    vertId parent1Id;
    vertId parent2Id;
    for (vertId parent_id : pn.parents) {
        const ProofNode& parent_pn = p.nodeLookup.at(parent_id);
        if (!match_parent1 && parent_pn.formula == pn.formula.members[1]) {
            match_parent1 = true;
            parent1Id = parent_id;
        } else if (!match_parent2 && parent_pn.formula == pn.formula.members[2]) {
            match_parent2 = true;
            parent2Id = parent_id;
        }
    }

    const bool result = match_parent1 && match_parent2;

    // Update assumptions with union
    if (result) {
        assumptions[vertex_id] = assumptions[parent1Id];
        assumptions[vertex_id].insert(assumptions[parent2Id].begin(), assumptions[parent2Id].end());
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

inline bool is_not_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 2 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "not";
}

bool verifyNotIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_not_vertex(pn)) {
        return false;
    }

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    bool formula_found = false;
    vertId formula_id = 0;
    // Make sure the current formula is a negation of an assumption
    for (const vertId a : assumptions[parents[0]]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            formula_found = true;
            formula_id = a;
            break;
        }
    }
    if (!formula_found) {
        for (const vertId a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                formula_found = true;
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_found) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id);
    }

    return formula_found;
}

bool verifyNotElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<vertId> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    bool formula_found = false;
    vertId formula_id = 0;
    // Make sure the current formula is a positive of an assumption
    for (const vertId a : assumptions[parents[0]]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
            formula_found = true;
            formula_id = a;
            break;
        }
    }
    if (!formula_found) {
        for (const vertId a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
                formula_found = true;
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_found) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id);
    }

    return formula_found;
}

inline bool is_equals_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "=";
}

bool verifyEqualsIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

bool verifyEqualsElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

inline bool is_forall_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "forall";
}

bool verifyForallIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

bool verifyForallElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

inline bool is_exists_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "exists";
}

bool verifyExistsIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

bool verifyExistsElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

// Verify that vertex is justified and update assumptions
bool verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);
    bool result = false;
    switch (pn.justification) {
        case Assume:
            assumptions[vertex_id] = {vertex_id};
            return true;
        case AndIntro:
            return verifyAndIntro(p, vertex_id, assumptions);
        case AndElim:
            return verifyAndElim(p, vertex_id, assumptions);
        case OrIntro:
            return verifyOrIntro(p, vertex_id, assumptions);
        case OrElim:
            return verifyOrElim(p, vertex_id, assumptions);
        case NotIntro:
            return verifyNotIntro(p, vertex_id, assumptions);
        case NotElim:
            return verifyNotElim(p, vertex_id, assumptions);
        case IfIntro:
            return verifyIfIntro(p, vertex_id, assumptions);
        case IfElim:
            return verifyIfElim(p, vertex_id, assumptions);
        case IffIntro:
            return verifyIffIntro(p, vertex_id, assumptions);
        case IffElim:
            return verifyIffElim(p, vertex_id, assumptions);
        case EqIntro:
            return verifyEqualsIntro(p, vertex_id, assumptions);
        case EqElim:
            return verifyEqualsElim(p, vertex_id, assumptions);
        case ForallIntro:
            return verifyForallIntro(p, vertex_id, assumptions);
        case ForallElim:
            return verifyForallElim(p, vertex_id, assumptions);
        case ExistsIntro:
            return verifyExistsIntro(p, vertex_id, assumptions);
        case ExistsElim:
            return verifyExistsElim(p, vertex_id, assumptions);
        default:
            std::cout << "Unknown Justification" << std::endl;
            break;
    }

    return result;
}

//Returns the file path that needs to be read
const char* VerifierInit(int argc, char** argv){
    // If no arguments are passed, print help
    if (argc < 2) {
        std::cout << "Usage: ./verif.exe [hyperslate_file]" << std::endl;
        exit(1);
    }
    // Parse and create hypergraph on each rank
    return argv[1];
}