
#include<cmath>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<list>
#include<numeric>

#include<mpi.h>

#include "../Proof/Proof.hpp"
#include "../SharedVerifier/SharedVerifier.hpp"

#include "LibParallelVerifier.hpp"
#include "MPIUtil.hpp"

using MPIUtil::getRankSizes;
using MPIUtil::getRankDisplacements;

// Parallel Verification & associated helpers =================================

bool ParallelVerifier::verifyParallelNoOpt(const Proof& p, LayerMapper mapper){
    auto [layerMap, depthMap] = mapper(p);
    
    //If there are nodes outside the depth map, they don't follow from
    //assumptions and the proof is invalid
    if(depthMap.size() != p.nodeLookup.size()) 
        return false;

    Assumptions assumptions;
    LayerMap::const_iterator layerIter = layerMap.begin();
    for (int layer = 0; layerIter != layerMap.end(); layer++, layerIter++){
        const std::unordered_set<VertId>& layerNodes = *layerIter;
        const size_t layerSize = layerNodes.size();
        std::vector<int> rankSizes = getRankSizes(layerSize);
        std::vector<int> rankDisplacements = getRankDisplacements(rankSizes);

        //The number of nodes this rank will verify
        int myNodeCount = rankSizes[myRank];            
        int myDisplacement = rankDisplacements[myRank];

        std::vector<uint8_t> verifiedSendBuf(myNodeCount);
        std::vector<uint8_t> verifiedReceiveBuf(layerSize);
        
        using NodeIterator = std::unordered_set<VertId>::const_iterator;
        NodeIterator nodeIter = layerNodes.begin();
        std::advance(nodeIter, myDisplacement);

        std::list<VertId> myNodes;
        for(size_t i = 0; i < myNodeCount; i++, nodeIter++){
            myNodes.push_back(*nodeIter);
            verifiedSendBuf[i] = \
            (uint8_t)SharedVerifier::verifyVertex(p, *nodeIter, assumptions);
            if(!verifiedSendBuf[i]){
                std::cout<<"Layer: "<<layer << ", Node:"<<*nodeIter<<std::endl; 
            }
        }

        MPI_Allgatherv((void*)verifiedSendBuf.data(), myNodeCount,\
                       MPI_UINT8_T, verifiedReceiveBuf.data(),\
                       rankSizes.data(), rankDisplacements.data(),\
                       MPI_UINT8_T, MPI_COMM_WORLD);
        
        for(size_t i = 0; i < layerSize; i++){
            if(!verifiedReceiveBuf[i]){
                return false;
            }
        }

        //Assumption Updates --------------------------------------------------

        if (layer != layerMap.size()-1){ //Ignore updates on final layer
            break;
        }
        
        //Serialization of assumptions
        //Passes assumption updates as a list of ints where where each node 
        //to update is a sequence starting with the node to update, followed
        //by all its assumptions, followed by -1 as a delimiter
        //Example: 5 1 2 3 -1 7 1 3 -1 6 2 -1    N=node A=assumption
        //         N A A A  D N A A  D N A  D    D=Delimiter
        int asumptSerialSize = 0;
        for(const VertId node : myNodes){
            asumptSerialSize += assumptions[node].size() + 2;
        }
        std::vector<int> assumptSerialization(asumptSerialSize);
        for(const VertId node : myNodes){
            assumptSerialization.push_back(node);
            for(const VertId assumption : assumptions[node]){
                assumptSerialization.push_back(assumption);
            }
            assumptSerialization.push_back(-1);
        }
        
        std::vector<int> assumptionSizes(numRanks);
        MPI_Allgather((void*)&asumptSerialSize, 1, MPI_INT,\
                      (void*)assumptionSizes.data(), 1,\
                      MPI_INT, MPI_COMM_WORLD);

        int assumptionSizeSum = \
            std::accumulate(assumptionSizes.begin(), assumptionSizes.end(), 0);
        std::vector<int> displacements = getRankDisplacements(assumptionSizes);

        std::vector<int> assumptionUpdates(assumptionSizeSum);
        MPI_Allgatherv((void*)assumptSerialization.data(), asumptSerialSize,\
                       MPI_INT, (void*)assumptionUpdates.data(), \
                       assumptionSizes.data(), displacements.data(), 
                       MPI_INT, MPI_COMM_WORLD);

        //Decode the serialized assumption updates and apply them
        VertId updatedNode = assumptionUpdates[0];
        for(int i = 1; i < assumptionUpdates.size(); i++){
            if(assumptionUpdates[i] == -1){
                updatedNode = assumptionUpdates[i+1]; 
            }else{
                assumptions[updatedNode].insert(assumptionUpdates[i+1]);
            }
        }
    }

    return true;
}

bool ParallelVerifier::verifyParallelSemanticJump(const Proof& proof,
                                                  LayerMapper mapper){
    return false;
}
