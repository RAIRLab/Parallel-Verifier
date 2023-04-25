

#include<mpi.h>
#include"../Proof/Proof.hpp"

#include"LibMPIVerifier.hpp"
#include"MPIUtil.hpp"

using MPIUtil::getRankSizes;
using MPIUtil::getRankDisplacements;

// Takes in a proof, a nodeId, and a depthMap, returns true if all of a nodes 
// parents are in the depth map 
bool allParentsInDepthMap(const Proof& p, VertId node,
                          const DepthMap& depthMap){
    const Proof::Node& potMember = p.nodeLookup.at(node);
    for(const VertId parent : potMember.parents){
        if(depthMap.find(parent) == depthMap.end()){
            return false;
        }
    }
    return true;
}

//Takes a proof and returns a map from layer numbers to sets of vertices
//on that layer.
//Works by climbing down the proof tree from the assumptions 
//Because it starts at assumptions, the map is not guaranteed to contain
//every node in P if the node is stand alone without any assumptions as
//a parent, however these cases only occur is a proof is false and
//will not be verified
std::pair<LayerMap, DepthMap> ParallelVerifier::getLayerMapMPI(const Proof& p){
    
    //Maps each node (VertID) to its layer number (size_t)
    DepthMap depthMap; 
    //Maps each layer number (size_t) to the set of nodes
    //(std::unordered_set<VertId>) on that layer
    LayerMap layerMap; 

    //Stores the nodes that are possibly members of the current layer
    std::unordered_set<VertId> potentialLayerMembers;

    //Add all assumptions to the depth map at level 0
    //and their children as possible members of layer 1
    layerMap.push_back(std::unordered_set<VertId>());
    for(const VertId assumption : p.assumptions){
        depthMap.insert({assumption, 0});
        layerMap.back().insert(assumption);
        for(VertId assumptionChild : p.nodeLookup.at(assumption).children){
            potentialLayerMembers.insert(assumptionChild);
        }
    }

    //While there are nodes that could potentially be members of the current
    //layer (stored within potentialLayerMembers) split them up among each rank
    //and determine if they actually are a member of the current layer
    //by if they are fully marked (all of their parents are in the depthmap)
    //if so they can be added to the current layer and their children made
    //contenders for the next layer. 
    size_t currentLayer = 1;
    while(potentialLayerMembers.size() != 0){
        
        //Figure out what our rank is handling on this iteration
        size_t numItems = potentialLayerMembers.size();
        std::vector<int> rankSizes = getRankSizes(numItems);
        std::vector<int> rankDisplacements = getRankDisplacements(rankSizes);
        int myItemCount = rankSizes[myRank];
        int myDisplacement = rankDisplacements[myRank];
        
        //Buffers for storing the MPI communications 
        //uint_8t used as a stand-in for bool so it plays nice
        //with MPI and I don't need to deal with vector<bool> drama
        std::vector<uint8_t> inLayerSendBuf(myItemCount);
        std::vector<uint8_t> inLayerReceiveBuf(numItems);

        using PotentialMemberIterator = \
              std::unordered_set<VertId>::const_iterator;
        PotentialMemberIterator iter = potentialLayerMembers.begin();
        PotentialMemberIterator endIter = potentialLayerMembers.end();
        std::vector<VertId> potentialMembersCopy(iter, endIter);

        //Advance to the start of the members this rank will be checking
        std::advance(iter, myDisplacement);

        //Compute if a node is on a layer by checking if all its
        //parents are in the depth map
        for(size_t i = 0; i < myItemCount && iter != endIter; i++, iter++){
            inLayerSendBuf[i] = 
                (uint8_t)allParentsInDepthMap(p, *iter, depthMap);
        }

        //Share which of our nodes are on the current layer
        MPI_Allgatherv((void*)inLayerSendBuf.data(), myItemCount,\
                      MPI_UINT8_T, (void*)inLayerReceiveBuf.data(),\
                      rankSizes.data(), rankDisplacements.data(),\
                      MPI_UINT8_T, MPI_COMM_WORLD);

        //Update our maps and mark the children of completed nodes
        //as being possible nodes on the next layer
        layerMap.push_back(std::unordered_set<VertId>());
        for(int i = 0; i < numItems; i++){
            VertId completed = potentialMembersCopy[i];
            if(inLayerReceiveBuf[i]){
                depthMap.insert({completed, currentLayer});
                layerMap.back().insert(completed);
                potentialLayerMembers.erase(completed);
                for(VertId compChild : p.nodeLookup.at(completed).children){
                    potentialLayerMembers.insert(compChild);
                }
            }
        }
        currentLayer++;
    }

    return std::make_pair(layerMap, depthMap);
}