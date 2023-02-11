

#pragma once

#include<unordered_set>
#include<unordered_map>
#include<optional>

#include"../SExpression/SExpression.hpp"

typedef int vertId;
typedef std::optional<vertId> optVertId;

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
        vertId id;
        sExpression formula;
        Justification justification;
        std::unordered_set<vertId> parents;
        std::unordered_set<vertId> children;
        SymbolTypeMap symbolTypeLookup;

        // Constructor
        Node(int vertId, sExpression formula, Justification justification);
        Node() = default;
    };


    //Representation ====================================================== 
    /*
        O(1) access to node data based on vertId as index
        This prevents us from using pointers on Proof::Nodes which would not translate between ranks.
        Allows proofs to easily be serialized for message passing.
    */
    std::unordered_map<vertId, Node> nodeLookup; 
    //We need to know our initial assumptions for our algs
    std::unordered_set<vertId> assumptions;

    // Methods ================================================================
    Proof() = default;
    Proof(std::string filename);
    std::string toString() const;
};

