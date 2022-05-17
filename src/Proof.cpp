
#include"Proof.hpp"

//This lets us construct the proof from the raw .slt file contents
Proof::Proof(std::string slateFileContents)
:Proof(parseHyperslateFile(slateFileContents))
{}

Proof::Proof(HyperslateFileData fileData){
    nodeLookup = std::unordered_map<vertId, ProofNode>();
    for(HyperslateDescription descriptionNode : fileData.descriptions){
        if(descriptionNode.formula != ""){  //Ignore selmers comment nodes
            ProofNode proofNode(
                descriptionNode.id,
                sExpression(descriptionNode.formula),
                descriptionNode.justification
            );
            if(proofNode.justification == HyperslateJustification::Assume)
                assumptions.insert(proofNode.id);
            nodeLookup[proofNode.id] = proofNode;
        }
    }
    for(HyperslateStructure connection : fileData.structures){
        for(vertId premise : connection.premises){
            nodeLookup[premise].children.insert(connection.conclusion);
            nodeLookup[connection.conclusion].parents.insert(premise);
        }
    }
}

std::unordered_set<std::string> reservedWords = {
    "and", "or", "iff", "imp", "not",
    "forall", "exists"
};

SymbolTypeMap compute_symbol_types(sExpression& formula, SymbolTypeMap& intermediateResult, bool predicateFound = false) {
    // TODO: Should we do anything if we find conflicting symbol types?

    /*
        Base cases
    */
    // (1) Formula is a symbol
    if (formula.type == sExpression::Type::Symbol) {
        intermediateResult[formula] = (predicateFound)?
            sExpressionSymbolType::ConstFreeVar:
            sExpressionSymbolType::Predicate;
        return intermediateResult;
    }
    // (2) Formula is not a list
    if (formula.type != sExpression::Type::List) {
        return intermediateResult;
    }
    // (3) Formula is a list of size 0 (a sad non-formula)
    if (formula.members.size() == 0) {
        return intermediateResult;
    }
    // (4) Formula is a list of size 1
    sExpression first = formula.members.at(0);
    if (formula.members.size() == 1) {
        intermediateResult[first] = (predicateFound)?
            sExpressionSymbolType::ConstFreeVar:
            sExpressionSymbolType::Predicate;
        return intermediateResult;
    }

    /*
     Recursive Cases:
     sExpression is a list greater than 1 member
    */

    // Assign the first element
    // unless its a reserved word
    std::unordered_set<std::string>::const_iterator containsReservedWord = reservedWords.find(first.value);
    if (containsReservedWord == reservedWords.end()) {
        intermediateResult[first] = (predicateFound)?
            sExpressionSymbolType::ConstFreeVar:
            sExpressionSymbolType::Predicate;
        predicateFound = true;
    }
    // Case 1: Formula is a quantifier
    // Mark bound variable
    if ((first.value == "forall" || first.value == "exists") &&
        formula.members.size() == 3
    ) {
        sExpression second = formula.members.at(1);
        intermediateResult[second] = sExpressionSymbolType::BoundVar;
        sExpression third = formula.members.at(2);
        return compute_symbol_types(third, intermediateResult, predicateFound);
    }

    // Case 2: Generic non-quantifier formula
    // Recursive on all other members
    for (auto it = formula.members.begin() + 1; it != formula.members.end(); ++it) {
        compute_symbol_types(*it, intermediateResult, predicateFound);
    }

    return intermediateResult;
}

SymbolTypeMap compute_symbol_types(sExpression& formula) {
    SymbolTypeMap intermediateResult;
    return compute_symbol_types(formula, intermediateResult, true);
}

ProofNode::ProofNode(int vertid, sExpression f, HyperslateJustification j) {
    id = vertid;
    formula = f;
    justification = j;
    symbolTypeLookup = compute_symbol_types(formula);
}
