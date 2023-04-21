
#include<unordered_map>
#include"SharedVerifier.hpp"
#include"timing.hpp"
#include"InferenceRules/rules.hpp"

using namespace SharedVerifier;

// Misc Shared Code ===========================================================

//Returns the file path that needs to be read
const char* SharedVerifier::init(int argc, char** argv){
    // If no arguments are passed, print help
    if (argc < 2) {
        std::cout << "Usage: ./verif.exe <ProofFile> <ProverSpecificArgs...>"\
        << std::endl;
        exit(1);
    }
    // Parse and create hypergraph on each rank
    return argv[1];
}


// Timings --------------------------------------------------------------------

static uint64_t startTime; //Uh oh a global

void SharedVerifier::startClock(){
    startTime = clock_now();
}

std::pair<double, uint64_t> SharedVerifier::endClock(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    double secs = totalCycles/(double)clockFreq;
    return std::make_pair(secs, totalCycles);
}

void SharedVerifier::endClockPrint(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    double secs = totalCycles/(double)clockFreq;
    printf("%lf Seconds, %ld Clock Cycles", secs, totalCycles);
}

uint64_t SharedVerifier::endClockCycles(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    return totalCycles;
}

//Returns the number of seconds elapsed as a double since
// the last call to startClock
double SharedVerifier::endClockSeconds(){
    uint64_t endTime = clock_now();
    uint64_t totalCycles = endTime-startTime;
    double secs = totalCycles/(double)clockFreq;
    return secs;
}

//Inference Rule Verification =================================================

//Struct to store verification functions
struct VerificationFunctions{
    bool (*syntax)(const Proof&, const VertId);
    bool (*semantics)(const Proof&, const VertId, Assumptions&);
};

//Macro to generate rule table members, they all follow the same naming scheme
#define INTRO_ELIM_RULES(N) \
{Proof::Justification:: N##Intro,\
 VerificationFunctions{verify##N##IntroSyntax, verify##N##IntroSemantics}},\
{Proof::Justification:: N##Elim,\
 VerificationFunctions{verify##N##ElimSyntax, verify##N##ElimSemantics}}

//Rule Lookup table
using RuleMap = \
    std::unordered_map<Proof::Justification, VerificationFunctions>;

const RuleMap rules = {
    {
        Proof::Justification::Assume,
        VerificationFunctions{verifyAssumptionSyntax,verifyAssumptionSemantics}
    },
    INTRO_ELIM_RULES(And),
    INTRO_ELIM_RULES(Or),
    INTRO_ELIM_RULES(If),
    INTRO_ELIM_RULES(Iff),
    INTRO_ELIM_RULES(Not),
    //INTRO_ELIM_RULES(Eq),
    //INTRO_ELIM_RULES(Forall),
    //INTRO_ELIM_RULES(Exists)
};

const VerificationFunctions& getRuleVerifiers(const Proof& p,
                                              const VertId vertexId){
    const Proof::Node& pn = p.nodeLookup.at(vertexId);
    RuleMap::const_iterator rule = rules.find(pn.justification);
    if(rule == rules.end()) //If it doesn't exist, throw error
        throw std::runtime_error("Verification Error: Unknown Rule");
    return rule->second;
}

bool SharedVerifier::verifyVertexSyntax(const Proof& p, const VertId vertexId){
    return getRuleVerifiers(p, vertexId).syntax(p, vertexId);
}

bool SharedVerifier::verifyVertexSemantics(const Proof& p,
                                           const VertId vertexId,
                                           Assumptions& assumptions){
    return getRuleVerifiers(p, vertexId).semantics(p, vertexId, assumptions);
}

// Verify that vertex is justified and update assumptions
bool SharedVerifier::verifyVertex(const Proof& p, const VertId vertexId,
                                  Assumptions& assumptions){
    const VerificationFunctions& verifiers = getRuleVerifiers(p, vertexId);
    return verifiers.syntax(p, vertexId) &&\
           verifiers.semantics(p, vertexId, assumptions);
}



//Vector version of layer map
using LayerVector = std::vector<std::unordered_set<VertId>>;


//Recursive helper for getLayerAndDepthMapsSerial
//Returns the depth of a node id, in a proof p, using and modifying
//a global depth map for the proof.
size_t SharedVerifier::getNodeDepth(const Proof& p, VertId id, DepthMap& depthMap){
    //Base case 1, we already know the depth of the node
    DepthMap::const_iterator depthMapIter = depthMap.find(id);
    if(depthMapIter != depthMap.end()){
        return depthMapIter->second;
    }
    //Base case 2, we're an assumption
    std::unordered_set<VertId> parents = p.nodeLookup.at(id).parents;
    if(parents.size() == 0){
        depthMap[id] = 0;

        return 0;
    }

    std::unordered_set<int>::const_iterator itr = parents.begin();
    size_t maxDepth = getNodeDepth(p, *itr, depthMap);
    itr++;
    for(;itr != parents.end(); itr++){
        size_t curDepth = getNodeDepth(p, *itr, depthMap);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }
    depthMap[id] = maxDepth + 1;
    return maxDepth + 1;
}

//O(n) serial construction of the depth and layer maps for a proof.
//n is the number of nodes in the proof
std::pair<LayerMap, DepthMap> SharedVerifier::getLayerMap(const Proof& p){
    //Depthmap construction is O(n) via dynamic programming
    DepthMap depthMap;
    size_t maxDepth = 0;
    for(const auto& [id, node] : p.nodeLookup){
        size_t curDepth = getNodeDepth(p, id, depthMap);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }

    //LayerMap construction O(n)
    //std::list fill constructor
    LayerVector layerVector(maxDepth + 1, std::unordered_set<VertId>());
    for(const auto& [id, depth] : depthMap){
        layerVector[depth].insert(id);
    }
    LayerMap layerMap(layerVector.begin(), layerVector.end());

    return std::make_pair(layerMap, depthMap);
}