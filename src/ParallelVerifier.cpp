
#include<cmath>
#include<unordered_map>
#include<unordered_set>
#include<memory>

#include<mpi.h>

#include "ProofIO/Proof.hpp"
#include "ProofIO/ProofIO.hpp"
#include "SharedVerifier/SharedVerifier.hpp"

//MPI globals
int myRank;
int numRanks;

using DepthMap = std::unordered_map<VertId, size_t>;

//Pseudo-Marking struct
struct Marking{
    VertId node;
    bool completed;
};

void MPIDebugPrint(std::string message, int rank = -1){
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == -1 || myRank == rank){
        std::cout<<"Rank "<<myRank<<":"<<message<<std::endl;
    }
}

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

//Takes in an an item count and determines
//how many items are optimally allocated to each rank
//O(numRanks)
std::vector<int> getRankSizes(int numItems){
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
std::vector<int> getRankDisplacements(const std::vector<int>& rankSizes){
    std::vector<int> displacements(rankSizes.size());
    displacements[0] = 0;
    for(int i = 1; i < rankSizes.size(); i++){
        displacements[i] = displacements[i-1] + rankSizes[i-1];
    }
    return displacements;
}


//Takes a proof and returns a map from vertices to 
//their maximum distance to an assumption 
//Climbs down the proof tree from the assumptions
DepthMap getDepthMap(const Proof& p){
    DepthMap depthMap;
    std::unordered_set<VertId> potentialLayerMembers;
    
    //Add all assumptions to the depth map at level 0
    // and add children as potential level 1
    for(const VertId assumption : p.assumptions){
        depthMap.insert({assumption, 0});
        for(VertId assumptionChild : p.nodeLookup.at(assumption).children){
            potentialLayerMembers.insert(assumptionChild);
        }
    }

    using PotentialMemberIterator = std::unordered_set<VertId>::const_iterator;
    size_t currentLayer = 1;
    while(potentialLayerMembers.size() != 0){
        
        size_t numItems = potentialLayerMembers.size();
        std::vector<int> rankSizes = getRankSizes(numItems);
        std::vector<int> rankDisplacements = getRankDisplacements(rankSizes);
        int myItemCount = rankSizes[myRank];
        int myDisplacement = rankDisplacements[myRank];
        
        //Buffers for storing the MPI communications 
        std::vector<int> vertIdSendBuf(myItemCount);
        std::vector<int> vertIdReceiveBuf(numItems);
        std::vector<uint8_t> inLayerSendBuf(myItemCount);
        std::vector<uint8_t> inLayerReceiveBuf(numItems);

        //MPIDebugPrint("Layer: " + std::to_string(currentLayer));

        PotentialMemberIterator iter = potentialLayerMembers.begin();
        PotentialMemberIterator endIter = potentialLayerMembers.end();

        //MPIDebugPrint("Before Advance:");

        std::advance(iter, myDisplacement);

        //MPIDebugPrint("Offset: " + std::to_string(myDisplacement));

        //Compute if a node is on a layer in parallel
        for(size_t i = 0; i < myItemCount && iter != endIter; i++, iter++){
            vertIdSendBuf[i] = *iter;
            inLayerSendBuf[i] = 
                (uint8_t)allParentsInDepthMap(p, *iter, depthMap);
        }

        //Share the information about who's on the current layer
        MPI_Allgatherv((void*)vertIdSendBuf.data(), myItemCount,\
                      MPI_INT, (void*)vertIdReceiveBuf.data(), \
                      rankSizes.data(), rankDisplacements.data(),\
                      MPI_INT, MPI_COMM_WORLD);
        MPI_Allgatherv((void*)inLayerSendBuf.data(), myItemCount,\
                      MPI_UINT8_T, (void*)inLayerReceiveBuf.data(),\
                      rankSizes.data(), rankDisplacements.data(),\
                      MPI_UINT8_T, MPI_COMM_WORLD);

        for(int i = 0; i < numItems; i++){
            VertId completed = vertIdReceiveBuf[i];
            if(inLayerReceiveBuf[i]){
                depthMap.insert({completed, currentLayer});
                potentialLayerMembers.erase(completed);
                for(VertId compChild : p.nodeLookup.at(completed).children){
                    potentialLayerMembers.insert(compChild);
                }
            }
        }
        currentLayer++;
    }

    if(myRank == 0){
        for(const auto[id, depth] : depthMap){
            std::cout<<id<<" "<<depth<<std::endl;
        }
    }

    return depthMap;
}

bool verifyParallel(const Proof& p){
    DepthMap depthMap = getDepthMap(p);
    if(depthMap.size() != p.nodeLookup.size())
        return false;

    return true;
}

std::string getFileContentsMPI(const char* filePath){
    MPI_File fileHandle;
    MPI_File_open(MPI_COMM_WORLD, filePath, MPI_MODE_RDONLY, MPI_INFO_NULL, &fileHandle);
    MPI_Offset fileSize;
    MPI_File_get_size(fileHandle, &fileSize);
    char* fileContentsBuff = new char[fileSize];
    MPI_File_read(fileHandle, fileContentsBuff, fileSize, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&fileHandle);
    std::string fileContents = std::string(fileContentsBuff);
    delete[] fileContentsBuff;
    return fileContents;
}

int main(int argc, char** argv){
    //MPI Setup
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    
    //Load Proof
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    std::string proofFileContents = getFileContentsMPI(proofFilePath);
    Proof proof = ProofIO::loadFromJSONContents(proofFileContents);

    //Run Verifier
    SharedVerifier::startClock();
    bool result = verifyParallel(proof);
    auto [seconds, cycles] = SharedVerifier::endClock();
    if(myRank == 0){
        std::cout<<seconds<<" Seconds, "<<cycles<<" Clock Cycles"<<std::endl;
    }

    MPI_Finalize();
    return !result;
}