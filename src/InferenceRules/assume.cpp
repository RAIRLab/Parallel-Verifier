
#pragma once
#include"../Proof.hpp"
#include"../SharedVerifier.hpp"


bool verifyAssumption(const Proof& p, vertId vertex_id, Assumptions& assumptions) {
    assumptions[vertex_id] = {vertex_id};
    return true;
}