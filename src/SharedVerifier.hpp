
#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "Proof.hpp"

#define MIN(a,b) (((a)<(b))?(a):(b))

using Markings = std::unordered_map<vertId, std::unordered_set<vertId>>;
using Assumptions = std::unordered_map<vertId, std::unordered_set<vertId>>;

const char* VerifierInit(int argc, char** argv);

//Markings utils
void mark(Markings& markings, vertId source, vertId dest);
bool hasCompleteMarkings(const Proof& p, vertId vertex_id, const std::unordered_set<vertId>& markingList);

//Infrence Rule Verification and helpers
bool verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions);
