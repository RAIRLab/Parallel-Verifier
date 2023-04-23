
#ifdef DEBUG
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

#ifdef DEBUG
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
            rankFailed = rankFailed ||
                !SharedVerifier::verifyVertex(p, *nodeIter, assumptions);
#ifdef DEBUG
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
#ifdef DEBUG
            MPIUtil::debugPrint("Verification failed on layer" + std::to_string(layer), 0);
#endif
            return false;
        }

        //Assumption Updates --------------------------------------------------

        if (layer == layerMap.size()-1){ //Ignore updates on final layer
            break;
        }
        
        //Serialization of assumptions
        //Passes assumption updates as a list of ints where where each node 
        //to update is a sequence starting with the node to update, followed
        //by all its assumptions, followed by -1 as a delimiter
        //Example: 5 1 2 3 -1 7 1 3 -1 6 2 -1    N=node A=assumption
        //         N A A A  D N A A  D N A  D    D=Delimiter
        
        std::vector<int> assumptSerialization;
        for(const VertId node : myNodes){
            assumptSerialization.push_back(node);
            for(const VertId assumption : assumptions[node]){
                assumptSerialization.push_back(assumption);
            }
            assumptSerialization.push_back(-1);
        }
        int asumptSerialSize = assumptSerialization.size();

#ifdef DEBUG
        //Print out our assumption vectors we're passing
        std::string assumptSerializationStr = "";
        for(size_t i = 0; i < assumptSerialization.size(); i++){
            assumptSerializationStr += std::to_string(assumptSerialization[i]) + " ";
        }
        std::cout << "Rank " << myRank << " Layer " << layer <<\
        " My Contributions: " << assumptSerializationStr << std::endl;
#endif
        
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

#ifdef DEBUG
        //Print out our assumption updates
        std::string assumptionUpdatesStr = "";
        for(size_t i = 0; i < assumptionUpdates.size(); i++){
            assumptionUpdatesStr += std::to_string(assumptionUpdates[i]) + " ";
        }
        std::cout << "Rank " << myRank << " Layer " << layer <<\
        " All Updates: " << assumptionUpdatesStr << std::endl;
#endif

        //Decode the serialized assumption updates and apply them
        VertId updatedNode = assumptionUpdates[0];
        for(int i = 1; i < assumptionUpdates.size(); i++){
            if(assumptionUpdates[i] == -1){
                updatedNode = assumptionUpdates[i+1]; 
                i++;
            }else{
                assumptions[updatedNode].insert(assumptionUpdates[i]);
            }
        }
    }

    return true;
}