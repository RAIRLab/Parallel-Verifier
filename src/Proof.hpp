

#pragma once

#include<unordered_set>
#include"SExpression.hpp"
#include"hyperslateUtils.hpp"

struct ProofNode{
    id_t id;
    HyperslateJustification justification;
    sExpression formula;
    std::unordered_set<id_t> children; //We only need children, not parents
};

struct Proof{
    //Reprsentation
    /*
        O(1) access to node data based on id_t as index
        This prevents us from using pointers on ProofNodes which would not translate between ranks.
        Allows proofs to easily be serialized for message passing.
    */
    std::vector<ProofNode> nodeLookup; 
    //We need to know our initial assumptions for our algs
    std::unordered_set<id_t> assumptions;
    
    //Methods
    Proof(HyperslateFileData proofData);
};