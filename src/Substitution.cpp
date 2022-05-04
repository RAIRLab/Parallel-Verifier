

#include"Substitution.hpp"


Substitution::Substitution() {
    variableTermMap = std::unordered_map<sExpression, sExpression>();
    // variableTermMap = std::map<sExpression, sExpression>();
}

// Get the list of variables that are the
// keys of the substitution
// std::vector<sExpression> Substitution::variables() {
std::unordered_set<sExpression> Substitution::variables() {
    std::unordered_set<sExpression> vars = std::unordered_set<sExpression>();
    // std::vector<sExpression> vars = std::vector<sExpression>();
    vars.reserve(variableTermMap.size());
    for (auto const& it : variableTermMap) {
        vars.insert(it.first);
        // vars.push_back(it.first);
    }
    return vars;
}

// Add new mapping from a variable to a term
// granted that the variable doesn't already
// exist in the substitution.
void Substitution::add(sExpression variable, sExpression term) {
    // Make sure it does not map to itself...
    // Nevermind...
    // if (variable == term) {
        // throw std::invalid_argument("Variable cannot map to itself");
    // }

    // Make sure variable is a variable...
    if (variable.type != sExpression::Type::Symbol) {
        throw std::invalid_argument("Variable argument must be a variable");
    }

    // Make sure variable isn't already in substitution set...
    if (variableTermMap.count(variable)) {
        throw std::invalid_argument("Variable " + variable.toString() + " already exists.");
    }

    // Perform an occurs check
    if (term.contains(variable)) {
        throw std::invalid_argument("Occurs Check");
    }

    // Add variable to set
    variableTermMap[variable] = term;
}

// Apply a substitution to a term
sExpression Substitution::apply(const sExpression& term) const {
    // If there is nothing in the substitution set, return the same term
    if (variableTermMap.size() == 0) {
        sExpression result = term;
        return result;
    }

    // Recurse down if the term is a FuncTerm
    if (term.type == sExpression::Type::List) {
        sExpression result = sExpression();
        result.type = sExpression::Type::List;
        result.members = std::vector<sExpression>();
        for (int i = 0; i < term.members.size(); i++) {
            result.members.push_back(this->apply(term.members[i]));
        }
        return result;
    }

    // If the term matches a variable, return the substitution
    for (auto const& it : variableTermMap) {
        if (it.first == term) {
            return it.second;
        }
    }

    return term;
}
