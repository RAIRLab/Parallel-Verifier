

#pragma once
#include"../Proof.hpp"
#include"../Substitution.hpp"
#include"../SharedVerifier.hpp"

inline bool is_forall_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "forall" && \
       pn.formula.members[1].type == sExpression::Type::Symbol;
}

// NOT DONE
bool verifyForallIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure node is a forall node
    if (!is_forall_vertex(pn)) {
        return false;
    }

    // Make sure we have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    // Grab bound variable
    const sExpression& bound_var = pn.formula.members[1];


    //TODO: Should not use try-catch for meaningful program flow.
    //We should replace this by making an sExpression iterator and having  
    //positionOf return an sExpression iterator to the position and return 
    //sExpression.end() if not found. This is how the standard library does this.

    // Find a position where the bound variable is used
    std::queue<uid_t> bound_var_pos = std::queue<uid_t>();
    try {
        bound_var_pos = pn.formula.positionOf(bound_var); 
    } catch (...) {
        // Bound variable is not in formula
        return false;
    }
    
    // TODO: Make sure it's not an already existing bound variable

    const vertId parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    // Find out its corresponding subterm in the parent formula
    sExpression oldSubterm; 
    try {
        oldSubterm = parent_pn.formula.atPosition(bound_var_pos);
    } catch (...) {
        // That position does not exist in the new formula
        return false;
    }

    // TODO: Check to make sure oldSubterm does not apepar
    // free in any in-scope assumptions


    // Create substitution
    Substitution sigma = Substitution();
    try {
        sigma.add(bound_var, oldSubterm);
    } catch (...) {
        // Any of the possible substitution errors
        // Likely occurs check
        return false;
    }

    // Check that oldTerm * sigma == newTerm
    bool result = sigma.apply(pn.formula) == parent_pn.formula;

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result; 
}

// NOT DONE
bool verifyForallElim(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    const vertId parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    // Make sure parent is a forall node
    if (!is_forall_vertex(parent_pn)) {
        return false;
    }

    // Grab bound variable
    const sExpression& bound_var = parent_pn.formula.members[1];

    //TODO: see comment in verifyForallIntro
    // Find a position where the bound variable is used
    std::queue<uid_t> bound_var_pos = std::queue<uid_t>();
    try {
        bound_var_pos = parent_pn.formula.positionOf(bound_var); 
    } catch (...) {
        // Bound variable is not in parent formula
        return false;
    }

    // Find out its corresponding subterm in the new formula
    sExpression newSubterm; 
    try {
        newSubterm = pn.formula.atPosition(bound_var_pos);
    } catch (...) {
        // That position does not exist in the new formula
        return false;
    }

    // TODO: Grab set of bound variables and make sure they're not in newSubterm

    // Create substitution
    Substitution sigma = Substitution();
    try {
        sigma.add(bound_var, newSubterm);
    } catch (...) {
        // Any of the possible substitution errors
        // Likely occurs check
        return false;
    }

    // Check that oldTerm * sigma == newTerm
    bool result = sigma.apply(parent_pn.formula) == pn.formula;


    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result; 
}