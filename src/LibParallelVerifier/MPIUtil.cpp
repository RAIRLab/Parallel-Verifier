


#include"LibParallelVerifier.hpp"
#include"MPIUtil.hpp"

#include<mpi.h>

#include<string>

int myRank;
int numRanks;

void MPIUtil::setGlobals(){
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
}

std::string MPIUtil::getFileContents(const char* filePath){
    MPI_File fileHandle;
    MPI_File_open(MPI_COMM_WORLD, filePath, MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &fileHandle);
    MPI_Offset fileSize;
    MPI_File_get_size(fileHandle, &fileSize);
    char* fileContentsBuff = new char[fileSize];
    MPI_File_read(fileHandle, fileContentsBuff, fileSize,
                  MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&fileHandle);
    std::string fileContents = std::string(fileContentsBuff, fileSize);
    delete[] fileContentsBuff;
    return fileContents;
}

// Debugging Helpers ==========================================================

void MPIUtil::debugPrint(std::string message, int rank){
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == -1 || myRank == rank){
        std::cout<<"Rank "<<myRank<<":"<<message<<std::endl;
    }
}

void MPIUtil::printLayerMap(const LayerMap& layerMap){
    if(myRank == 0){
        LayerMap::const_iterator iter = layerMap.begin();
        for(int i = 0; iter != layerMap.end(); iter++, i++){
            std::cout<<"Layer "<<i<<":";
            for(const auto id : *iter){
                std::cout<<id<<",";
            }
            std::cout<<"\b"<<std::endl;
        }
    }
}

// General MPI Helpers ========================================================

//Takes in an an item count and determines
//how many items are optimally allocated to each rank
//O(numRanks)
std::vector<int> MPIUtil::getRankSizes(int numItems){
    int baseSize = numItems / numRanks;
    std::vector<int> rankSizes(numRanks, baseSize);
    for(int i = 0; i < numItems % numRanks; i++){
        rankSizes[i]++;
    }
    return rankSizes;
}

//Takes in an rankSizes map and computes the displacements
//for MPI_AllGatherV
//O(numRanks)
std::vector<int> MPIUtil::getRankDisplacements(
                            const std::vector<int>& rankSizes){
    std::vector<int> displacements(rankSizes.size());
    displacements[0] = 0;
    for(int i = 1; i < rankSizes.size(); i++){
        displacements[i] = displacements[i-1] + rankSizes[i-1];
    }
    return displacements;
}