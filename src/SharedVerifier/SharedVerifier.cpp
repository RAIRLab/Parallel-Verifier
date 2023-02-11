
#include<unordered_map>
#include"SharedVerifier.hpp"
#include"timing.hpp"

using namespace SharedVerifier;

// Misc Shared Code ===========================================================

//Returns the file path that needs to be read
const char* SharedVerifier::init(int argc, char** argv){
    // If no arguments are passed, print help
    if (argc != 2) {
        std::cout << "Usage: ./verif.exe [ProofFile]" << std::endl;
        exit(1);
    }
    // Parse and create hypergraph on each rank
    return argv[1];
}


// Timings ----------------------------------------------------------------------

static uint64_t startTime; //Uh oh a global

void SharedVerifier::startClock(){
    startTime = clock_now();
}

void SharedVerifier::endClock(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    double secs = totalCycles/(double)clockFreq;
    printf("%lf Seconds, %ld Clock Cycles\n", secs, totalCycles);
}

//Inference Rule Verification =================================================

#include"InferenceRules/assume.hpp"
#include"InferenceRules/and.hpp"
#include"InferenceRules/or.hpp"
#include"InferenceRules/not.hpp"
#include"InferenceRules/if.hpp"
#include"InferenceRules/iff.hpp"
#include"InferenceRules/equals.hpp"
#include"InferenceRules/forall.hpp"
#include"InferenceRules/exists.hpp"

//Macro to generate rule table members since they all follow the same naming scheme
#define INTRO_ELIM_RULES(N) {Proof::Justification:: N##Intro,\
verify##N##Intro}, {Proof::Justification:: N##Elim, verify##N##Elim}

//Rule Lookup table
using VerificationFunction = bool(*)(const Proof&, vertId, Assumptions&);
using RuleMap = std::unordered_map<Proof::Justification, VerificationFunction>;
const RuleMap rules = {
    {Proof::Justification::Assume, verifyAssumption}, 
    INTRO_ELIM_RULES(And),
    INTRO_ELIM_RULES(Or),
    INTRO_ELIM_RULES(If),
    INTRO_ELIM_RULES(Iff),
    INTRO_ELIM_RULES(Eq),
    INTRO_ELIM_RULES(Forall),
    INTRO_ELIM_RULES(Exists)
};

// Verify that vertex is justified and update assumptions
bool SharedVerifier::verifyVertex(const Proof& p, vertId vertex_id, Assumptions& assumptions){
    const Proof::Node& pn = p.nodeLookup.at(vertex_id);
    //Lookup the rule, O(1)
    RuleMap::const_iterator rule = rules.find(pn.justification);        
    if(rule == rules.end()) //If it doesn't exist, throw error
        throw std::runtime_error("Verification Error: Unknown Rule");
    //Call the respective verification function
    return rule->second(p, vertex_id, assumptions);  
}

bool SharedVerifier::verifyVertexSyntax(const Proof& p, vertId vertex_id){
    return true;
}

bool SharedVerifier::verifyVertexSemantics(const Proof& p, vertId vertex_id, Assumptions& assumptions){
    return true;
}