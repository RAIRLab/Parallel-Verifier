
#include<unordered_map>
#include"SharedVerifier.hpp"

// Misc Shared Code ===========================================================================

//Returns the file path that needs to be read
const char* VerifierInit(int argc, char** argv){
    // If no arguments are passed, print help
    if (argc < 2) {
        std::cout << "Usage: ./verif.exe [hyperslate_file]" << std::endl;
        exit(1);
    }
    // Parse and create hypergraph on each rank
    return argv[1];
}

// Markings ====================================================================================

// Update dest's marking vector to contain source -Brandon
void mark(Markings& markings, vertId source, vertId dest) {
    Markings::iterator it = markings.find(dest);
    if (it == markings.end()) {
        markings[dest] = std::unordered_set<vertId>();
    }
    markings.at(dest).insert(source);
}

//Returns true if the node at vertex_id in proof p has a full list of markings
bool hasCompleteMarkings(const Proof& p, vertId vertex_id, const std::unordered_set<vertId>& markingList) {
    const std::unordered_map<HyperslateJustification, size_t> numMarkingsMap = {
        {Assume, 0}, {AndIntro, 2}, {AndElim, 1},
        {OrIntro, 1}, {OrElim, 3}, {NotIntro, 2},
        {NotElim, 2}, {IfIntro, 1}, {IfElim, 2},
        {IffIntro, 2}, {IffElim, 2}
    };
    size_t markingsNeeded = numMarkingsMap.at(p.nodeLookup.at(vertex_id).justification);
    return markingList.size() == markingsNeeded;
}

//Infrence Rule Verification ==============================================================

#include"InferenceRules/assume.cpp"
#include"InferenceRules/and.cpp"
#include"InferenceRules/or.cpp"
#include"InferenceRules/not.cpp"
#include"InferenceRules/if.cpp"
#include"InferenceRules/iff.cpp"
#include"InferenceRules/equals.cpp"
#include"InferenceRules/forall.cpp"
#include"InferenceRules/exists.cpp"

//Macro to generate rule table members since they all follow the same naming scheme
#define INTRO_ELIM_RULES(N) {N##Intro, verify##N##Intro}, {N##Elim, verify##N##Elim}

//Rule Lookup table
using VerificationFunction = bool(*)(const Proof&, vertId, Assumptions&);
using RuleMap = std::unordered_map<HyperslateJustification, VerificationFunction>;
const RuleMap rules = {
    {Assume, verifyAssumption}, 
    INTRO_ELIM_RULES(And),
    INTRO_ELIM_RULES(Or),
    INTRO_ELIM_RULES(If),
    INTRO_ELIM_RULES(Iff),
    INTRO_ELIM_RULES(Eq),
    INTRO_ELIM_RULES(Forall),
    INTRO_ELIM_RULES(Exists)
};

// Verify that vertex is justified and update assumptions
bool verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions){
    const ProofNode& pn = p.nodeLookup.at(vertex_id);
    RuleMap::const_iterator rule = rules.find(pn.justification);        //Lookup the rule, O(1)
    if(rule == rules.end())                                             //If it dosn't exist, throw error
        throw std::runtime_error("Verification Error: Unknown Rule");
    return rule->second(p, vertex_id, assumptions);                     //Call the respective verif function
}