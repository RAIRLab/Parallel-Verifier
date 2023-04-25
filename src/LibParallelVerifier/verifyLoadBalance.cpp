
#include <mpi.h>

#include "LibParallelVerifier.hpp"
#include "MPIUtil.hpp"

using MPIUtil::getRankSizes, MPIUtil::getRankDisplacements;
using SharedVerifier::verifyVertex, SharedVerifier::verifyVertexSemantics, \
 SharedVerifier::verifyVertexSyntax;

//Takes in a list of the nodes assigned for verification to each node 
//(rankSizes), a vector with a partial order where nodes are guaranteed to be  
//correctly ordered by layer (layerNodeQueue), and an index into this vector
//of the current position. It returns how many syntax checks should take place
//on each rank
/*
Examples:
    numRanks = 3
    getSyntaxCheckSizes = [1, 0, 0],
    layerNodeQueue = [1, 2, 3, 4, 5 , 6]
    syntaxCheckIndex = 1
    returns [0, 1, 1]

    numRanks = 3
    getSyntaxCheckSizes = [1, 0, 0],
    layerNodeQueue = [1]
    syntaxCheckIndex = 0
    returns [0, 1, 0]
*/
std::vector<int> getSyntaxCheckSizes(
 const std::vector<int>& rankSizes,
 const std::vector<VertId>& layerNodeQueue,
 const size_t syntaxCheckIndex){
    std::vector<int> syntaxCheckSizes(rankSizes.size(), 0);
    //first elem in rank sizes will always contain the max semantic check load
    const int maxSize = rankSizes[0];
    int tempSyntaxCheckIndex = syntaxCheckIndex;
    int startIndex = 1;
    bool modified; //If we have changed the syntaxCheckSizes this turn
    do{
        modified = false;
        for(int i = startIndex; i < rankSizes.size(); i++){
            const int dif = maxSize - (syntaxCheckSizes[i] + rankSizes[i]);
            if(dif != 0 && tempSyntaxCheckIndex < layerNodeQueue.size()){
                modified = true;
                tempSyntaxCheckIndex++;
                syntaxCheckSizes[i]++;
            }else{
                //Small optimization to avoid rechecking maxed out ranks
                startIndex++; 
            }
        }
    }while(modified);
    return syntaxCheckSizes;
}

//The actual load balancing parallel verifier
//Takes a proof p and a layerMap generator algorithm 
//and returns true if the proof is legal or false if not
bool ParallelVerifier::verifyParallelLoadBalance(const Proof& p,
 LayerMapper mapper){
    auto [layerMap, depthMap] = mapper(p);
    
    //If there are nodes outside the depth map, they don't follow from
    //assumptions and the proof is invalid
    if(depthMap.size() != p.nodeLookup.size()) 
        return false;

    //ordering matters here so we order everything in vectors
    std::vector<std::vector<VertId>> layers;
    for(const std::unordered_set<VertId>& layer : layerMap){
        layers.push_back(std::vector<VertId>(layer.begin(), layer.end()));
    }

    //This list and its associated syntaxCheckIndex
    //marks where the syntax check has completed up to
    std::vector<VertId> layerNodeQueue;
    for(const std::vector<VertId>& layer : layers){
        layerNodeQueue.insert(layerNodeQueue.end(), layer.begin(), layer.end());
    }
    size_t syntaxCheckIndex = 0;
    std::unordered_set<VertId> syntaxCheckedNodes;

    //Main loop over each layer
    Assumptions assumptions;
    for (int layerIndex = 0; layerIndex < layers.size(); layerIndex++){
        const std::vector<VertId>& layerNodes = layers[layerIndex];
        const size_t layerSize = layerNodes.size();
        std::vector<int> rankSizes = getRankSizes(layerSize);
        std::vector<int> rankDisplacements = getRankDisplacements(rankSizes);

        //The number of nodes this rank will full verify
        int myNodeCount = rankSizes[myRank];            
        int myDisplacement = rankDisplacements[myRank];

        //The number of syntaxChecks performed at the rank at an index
        //to Load balance with 
        std::vector<int> syntaxCheckSizes = getSyntaxCheckSizes( \
            rankSizes, layerNodeQueue, syntaxCheckIndex);
        std::vector<int> syntaxCheckDisplacements = \
            getRankDisplacements(syntaxCheckSizes);

        //The number of nodes this rank will syntax verify
        int syntaxNodeCount = syntaxCheckSizes[myRank];            
        int syntaxDisplacement = syntaxCheckDisplacements[myRank];

        //Verification --------------------------------------------------------

        //Do our full checks or semantic checks
        std::list<VertId> semanticChecked; //Nodes that need assumptions sent
        bool rankFailed = false;
        for(int i = 0; i < myNodeCount; i++){
            int index = myDisplacement + i;
            VertId node = layerNodes[index];
            semanticChecked.push_back(node);

            //If we've haven't been syntax checked do a full check
            //Otherwise if we have just do a semantic check
            bool success;
            std::unordered_set<VertId> newAssumptionIds;

            assumptions[layerNodes[index]] = std::unordered_set<VertId>();

            if(syntaxCheckedNodes.find(node) == syntaxCheckedNodes.end()){
                bool success = verifyVertex(p, layerNodes[index], assumptions, assumptions[layerNodes[index]]);
                rankFailed = rankFailed || !success;
            }else{
                bool success = verifyVertexSemantics(p, layerNodes[index], assumptions, assumptions[layerNodes[index]]);
                rankFailed = rankFailed || !success;;
            }

            if (rankFailed) {
                break;
            };
        }  

        //Do our forward syntax checks
        for(int i = 0; i < syntaxNodeCount; i++){
            int index = syntaxCheckIndex + syntaxDisplacement + i;
            rankFailed = rankFailed || \
             !verifyVertexSyntax(p, layerNodeQueue[index]);
        }

        //Syntax Check updates ------------------------------------------------
        //Compute who is getting syntax checked this layer on all ranks 
        std::unordered_set<VertId> nextSyntaxChecked;

        //At minimum every node on the current layer who is not already 
        //syntax checked will be syntax checked
        for (const VertId node : layerNodes){
            if(syntaxCheckedNodes.find(node) == syntaxCheckedNodes.end()){
                nextSyntaxChecked.insert(node);
            }
        }

        //In addition to nodes that checked early at the syntaxCheckIndex
        int numSyntaxChecked = rankDisplacements[rankDisplacements.size()-1];
        for(int i = 0; i < numSyntaxChecked; i++){
            nextSyntaxChecked.insert(layerNodeQueue[syntaxCheckIndex + i]);
        }

        syntaxCheckIndex += nextSyntaxChecked.size();
        syntaxCheckedNodes.merge(nextSyntaxChecked);
        
        //MPI reduction -------------------------------------------------------
        
        bool verificationFailure;
        MPI_Allreduce((void*)&rankFailed, (void*)&verificationFailure, \
         1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if(verificationFailure){
            return false;
        }

        //Assumption Updates --------------------------------------------------

        if (layerIndex == layerMap.size()-1){ //Ignore updates on final layer
            break;
        }
        
        ParallelVerifier::updateAssumptions(assumptions, semanticChecked);
    }

    return true;
}
