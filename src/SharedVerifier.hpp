

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "Proof.hpp"

#define CLOCK_RATE 512000000
#define MIN(a,b) (((a)<(b))?(a):(b))

using Markings = std::unordered_map<id_t, std::unordered_set<id_t>>;
using Assumptions = std::unordered_map<id_t, std::unordered_set<id_t>>;

const char* VerifierInit(int argc, char** argv);

//Markings utils
void mark(Markings& markings, id_t source, id_t dest);
bool hasCompleteMarkings(const Proof& p, id_t vertex_id, const std::unordered_set<id_t>& markingList);

//Infrence Rule Verification and helpers
bool verifyVertex(const Proof& p, id_t vertex_id, Assumptions& assumptions);

//And
bool is_and_vertex(const ProofNode& pn);
bool verifyAndIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions);
bool verifyAndElim(const Proof& p, id_t vertex_id, Assumptions& assumptions);

//Or
bool is_or_vertex(const ProofNode& pn);
bool verifyOrIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions);
bool verifyOrElim(const Proof& p, id_t vertex_id, Assumptions& assumptions);

//If
bool is_if_vertex(const ProofNode& pn);
bool verifyIfIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions);
bool verifyIfElim(const Proof& p, id_t vertex_id, Assumptions& assumptions);

//Iff
bool is_iff_vertex(const ProofNode& pn);
bool verifyIffIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions);
bool verifyIffElim(const Proof& p, id_t vertex_id, Assumptions& assumptions);

//Not
bool is_not_vertex(const ProofNode& pn);
bool verifyNotIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions);
bool verifyNotElim(const Proof& p, id_t vertex_id, Assumptions& assumptions);