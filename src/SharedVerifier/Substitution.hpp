
//Currently Dead Code, would be used for first order
/*
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <map>
#include <vector>

#include"../SExpression/SExpression.hpp"

struct Substitution {
    std::unordered_map<sExpression, sExpression> variableTermMap;
    // std::map<sExpression, sExpression> variableTermMap;
    Substitution(void);
    void add(sExpression variable, sExpression term);
    sExpression apply(const sExpression& term) const;
    std::unordered_set<sExpression> variables(void);
    // std::vector<sExpression> variables(void);
};

*/