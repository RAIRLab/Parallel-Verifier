
#ifdef PRINT_DEBUG
#include <iostream>
#endif 

#include<mpi.h>

#include "../Proof/Proof.hpp"
#include "../SharedVerifier/SharedVerifier.hpp"

#include "LibParallelVerifier.hpp"
#include "MPIUtil.hpp"

using MPIUtil::getRankSizes;
using MPIUtil::getRankDisplacements;

bool ParallelVerifier::verifyParallelNoOpt(const Proof& p, LayerMapper mapper){
    auto [layerMap, depthMap] = mapper(p);

    
    //If there are nodes outside the depth map, they don't follow from
    //assumptions and the proof is invalid
    if(depthMap.size() != p.nodeLookup.size()) 
        return false;

    Assumptions assumptions;
    LayerMap::const_iterator layerIter = layerMap.begin();
    for (int layer = 0; layerIter != layerMap.end(); layer++, layerIter++){

#ifdef PRINT_DEBUG
        std::cout<<"Rank " << myRank <<": Verifying layer: "<< layer << \
        "/" << layerMap.size() << " With assumptions: " << \
        SharedVerifier::assumptionsToString(assumptions) << std::endl;
#endif 
        const std::unordered_set<VertId>& layerNodes = *layerIter;
        const size_t layerSize = layerNodes.size();
        std::vector<int> rankSizes = getRankSizes(layerSize);
        std::vector<int> rankDisplacements = getRankDisplacements(rankSizes);

        //The number of nodes this rank will verify
        int myNodeCount = rankSizes[myRank];            
        int myDisplacement = rankDisplacements[myRank];

        using NodeIterator = std::unordered_set<VertId>::const_iterator;
        NodeIterator nodeIter = layerNodes.begin();
        std::advance(nodeIter, myDisplacement);
        
        bool rankFailed = false;
        std::list<VertId> myNodes; //List of nodes we need to update assumptions
        for(size_t i = 0; i < myNodeCount; i++, nodeIter++){
            myNodes.push_back(*nodeIter);
            auto [success, newAssumptionIds] = SharedVerifier::verifyVertex(p, *nodeIter, assumptions);
            rankFailed = rankFailed || !success;
            if (rankFailed) {
                break;
            }

            // Update assumptions
            assumptions[*nodeIter] = std::move(newAssumptionIds);

#ifdef PRINT_DEBUG
            if(rankFailed){
                std::cout<<"Failed on node: " << p.nodeLookup.at((VertId)*nodeIter).toString() << std::endl;
                std::cout<<"With assumptions: " << SharedVerifier::assumptionsToString(assumptions) << std::endl;
            }
#endif
        }

        bool verificationFailure;
        MPI_Allreduce((void*)&rankFailed, (void*)&verificationFailure, \
         1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if(verificationFailure){
#ifdef PRINT_DEBUG
            MPIUtil::debugPrint("Verification failed on layer" + std::to_string(layer), 0);
#endif
            return false;
        }

        //Assumption Updates --------------------------------------------------

        if (layer == layerMap.size()-1){ //Ignore updates on final layer
            break;
        }

        ParallelVerifier::updateAssumptions(assumptions, myNodes);
    }

    return true;
}