

#pragma once

#include<unordered_set>
#include<unordered_map>
#include"SExpression.hpp"
#include"hyperslateUtils.hpp"

struct ProofNode{
    vertId id;
    HyperslateJustification justification;
    sExpression formula;
    std::unordered_set<vertId> parents;
    std::unordered_set<vertId> children;
};

struct Proof{
    //Reprsentation
    /*
        O(1) access to node data based on vertId as index
        This prevents us from using pointers on ProofNodes which would not translate between ranks.
        Allows proofs to easily be serialized for message passing.
    */
    std::unordered_map<vertId, ProofNode> nodeLookup; 
    //We need to know our initial assumptions for our algs
    std::unordered_set<vertId> assumptions;
    
    //Methods
    Proof(HyperslateFileData proofData);
    Proof(std::string hyperslateFileString);
};