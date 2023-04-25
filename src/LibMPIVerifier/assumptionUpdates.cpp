
#include<mpi.h>
#include<vector>
#include<list>

#include "../SharedVerifier/SharedVerifier.hpp"

#include "LibMPIVerifier.hpp"
#include "MPIUtil.hpp"

using MPIUtil::getRankDisplacements;


/** MPI based assumption updating
 * @param assumptions: map of nodeIDs to their assumptions, this is modified
 * on all ranks with the new assumptions on each rank.
 * @param checked: list of nodeIDs that have been checked on the current rank
*/
void ParallelVerifier::updateAssumptions(Assumptions& assumptions,
                                         const std::list<VertId>& checked){
    //Serialization of assumptions
    //Passes assumption updates as a list of ints where where each node 
    //to update is a sequence starting with the node to update, followed
    //by all its assumptions, followed by -1 as a delimiter
    //Example: 5 1 2 3 -1 7 1 3 -1 6 2 -1    N=node A=assumption
    //         N A A A  D N A A  D N A  D    D=Delimiter
    
    std::vector<int> assumptSerialization;
    for(const VertId node : checked){
        assumptSerialization.push_back(node);
        for(const VertId assumption : assumptions[node]){
            assumptSerialization.push_back(assumption);
        }
        assumptSerialization.push_back(-1);
    }
    int asumptSerialSize = assumptSerialization.size();

#ifdef PRINT_DEBUG
    //Print out our assumption vectors we're passing
    std::string assumptSerializationStr = "";
    for(size_t i = 0; i < assumptSerialization.size(); i++){
        assumptSerializationStr += std::to_string(assumptSerialization[i]) + " ";
    }
    std::cout << "Rank " << myRank << " Layer ?" <<\
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

#ifdef PRINT_DEBUG
    //Print out our assumption updates
    std::string assumptionUpdatesStr = "";
    for(size_t i = 0; i < assumptionUpdates.size(); i++){
        assumptionUpdatesStr += std::to_string(assumptionUpdates[i]) + " ";
    }
    std::cout << "Rank " << myRank << " Layer ?" <<\
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