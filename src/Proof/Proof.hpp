

#pragma once

#include<unordered_set>
#include<unordered_map>
#include<optional>

#include"../SExpression/SExpression.hpp"

typedef int VertId;
typedef std::optional<VertId> optVertId;

struct Proof{

    // Namespace Members ====================================================== 

    enum class SymbolType {
        Predicate,
        Function,
        BoundVar,
        ConstFreeVar // I don't think we would be able to distinguish
    };

    using SymbolTypeMap = std::unordered_map<sExpression, Proof::SymbolType>;

    enum class Justification{
        Assume, AndIntro, AndElim, OrIntro, OrElim,
        NotIntro, NotElim, IfIntro, IfElim, IffIntro, IffElim,
        EqIntro, EqElim, ForallIntro, ForallElim,
        ExistsIntro, ExistsElim,
        COUNT //Number of elements
    };

    struct Node{
        VertId id;
        sExpression formula;
        Justification justification;
        std::unordered_set<VertId> parents;
        std::unordered_set<VertId> children;
        SymbolTypeMap symbolTypeLookup;

        // Constructor
        Node(int VertId, sExpression formula, Justification justification);
        Node() = default;
    };


    //Representation ==========================================================
     
    /*
        O(1) access to node data based on VertId as index
        This prevents us from using pointers on Proof::Nodes which would not translate between ranks.
        Allows proofs to easily be serialized for message passing.
    */
    std::unordered_map<VertId, Node> nodeLookup; 
    //We need to know our initial assumptions for our algs
    std::unordered_set<VertId> assumptions;

    // Methods ================================================================
    Proof() = default;
    Proof(std::string filename); //Uses ProofIO to construct
    std::string toString() const;
};

