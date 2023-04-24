
#include<mpi.h>

#include "../Proof/Proof.hpp"
#include "../SharedVerifier/SharedVerifier.hpp"

#include "LibParallelVerifier.hpp"
#include "MPIUtil.hpp"

using MPIUtil::getRankSizes;
using MPIUtil::getRankDisplacements;

bool ParallelVerifier::verifyParallelOriginal(const Proof& p,
                                              LayerMapper mapper){
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
            const auto [result, newAssumptions] = SharedVerifier::verifyVertex(p, *nodeIter, assumptions);
            if (result) {
                // Update assumptions
                for (auto [assumptionNode, assumptionIds] : newAssumptions) {
                    assumptions[assumptionNode] = assumptionIds;
                }
            }
            verifiedSendBuf[i] = (uint8_t)result;
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

        if (layer == layerMap.size()-1){ //Ignore updates on final layer
            break;
        }
        
        ParallelVerifier::updateAssumptions(assumptions, myNodes);
    }

    return true;
}