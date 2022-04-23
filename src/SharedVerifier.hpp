
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "Proof.hpp"

#define CLOCK_RATE 512000000
#define MIN(a,b) (((a)<(b))?(a):(b))

using Markings = std::unordered_map<vertId, std::unordered_set<vertId>>;
using Assumptions = std::unordered_map<vertId, std::unordered_set<vertId>>;

const char* VerifierInit(int argc, char** argv);

//Markings utils
void mark(Markings& markings, vertId source, vertId dest);
bool hasCompleteMarkings(const Proof& p, vertId vertex_id, const std::unordered_set<vertId>& markingList);

//Infrence Rule Verification and helpers
bool verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions);

//And
bool is_and_vertex(const ProofNode& pn);
bool verifyAndIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions);
bool verifyAndElim(const Proof& p, vertId vertex_id, Assumptions& assumptions);

//Or
bool is_or_vertex(const ProofNode& pn);
bool verifyOrIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions);
bool verifyOrElim(const Proof& p, vertId vertex_id, Assumptions& assumptions);

//If
bool is_if_vertex(const ProofNode& pn);
bool verifyIfIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions);
bool verifyIfElim(const Proof& p, vertId vertex_id, Assumptions& assumptions);

//Iff
bool is_iff_vertex(const ProofNode& pn);
bool verifyIffIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions);
bool verifyIffElim(const Proof& p, vertId vertex_id, Assumptions& assumptions);

//Not
bool is_not_vertex(const ProofNode& pn);
bool verifyNotIntro(const Proof& p, vertId vertex_id, Assumptions& assumptions);
bool verifyNotElim(const Proof& p, vertId vertex_id, Assumptions& assumptions);