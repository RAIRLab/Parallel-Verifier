
//Not supported in current version
/*
#pragma once
#include"../../ProofIO/Proof.hpp"
#include"../Substitution.hpp"
#include"../SharedVerifier.hpp"

inline bool is_exists_vertex(const Proof::Node& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "exists" && \
       pn.formula.members[1].type == sExpression::Type::Symbol;
}

bool verifyExistsIntro(const Proof& p, VertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

bool verifyExistsElim(const Proof& p, VertId vertex_id, Assumptions& assumptions) {
    return false; // TODO
}

*/