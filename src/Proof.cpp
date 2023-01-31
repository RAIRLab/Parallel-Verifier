
#include"Proof.hpp"


std::unordered_set<std::string> reservedWords = {
    "and", "or", "iff", "imp", "not",
    "forall", "exists"
};

SymbolTypeMap compute_symbol_types(sExpression& formula, SymbolTypeMap& intermediateResult, bool predicateFound = false) {
    // TODO: Should we do anything if we find conflicting symbol types?

    /* Base cases */
    // (1) Formula is a symbol
    if (formula.type == sExpression::Type::Symbol) {
        intermediateResult[formula] = (predicateFound)?
            Proof::SymbolType::ConstFreeVar:
            Proof::SymbolType::Predicate;
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
            Proof::SymbolType::ConstFreeVar:
            Proof::SymbolType::Predicate;
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
            Proof::SymbolType::ConstFreeVar:
            Proof::SymbolType::Predicate;
        predicateFound = true;
    }
    // Case 1: Formula is a quantifier
    // Mark bound variable
    if ((first.value == "forall" || first.value == "exists") &&
        formula.members.size() == 3
    ) {
        sExpression second = formula.members.at(1);
        intermediateResult[second] = Proof::SymbolType::BoundVar;
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

Proof::Node::Node(int vertId, sExpression f, Proof::Justification j) {
    id = vertId;
    formula = f;
    justification = j;
    symbolTypeLookup = compute_symbol_types(formula);
}
