
#pragma once

#include<vector>
#include"LibParallelVerifier.hpp"

extern int myRank;
extern int numRanks;

namespace MPIUtil{
    
    //Sets the MPI Globals myRank and numRanks
    void setGlobals();

    std::string getFileContents(const char* filePath);
    
    //Takes a message and a rank, prints only to the given rank, if rank is
    //given as -1, prints to all ranks.
    void debugPrint(std::string message, int rank = -1);

    //Prints a given LayerMap on rank 0
    void printLayerMap(const LayerMap& layerMap);

    //Takes in an rankSizes map and computes the displacements
    //for MPI_AllGatherV
    //O(numRanks)
    std::vector<int> getRankDisplacements(const std::vector<int>& rankSizes);

    //Takes in an an item count and determines
    //how many items are optimally allocated to each rank
    //O(numRanks)
    std::vector<int> getRankSizes(int numItems);
}