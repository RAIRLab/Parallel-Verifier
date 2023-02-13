
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
    size_t membersPerRank = (size_t)ceil(potentialLayerMembers.size() / \
                                         (double)numRanks);
    
    //Buffers for storing the MPI communications 
    std::unique_ptr<int[]> vertIdSendBuf(new int[membersPerRank]);
    std::unique_ptr<int[]> vertIdReceiveBuf(new int[membersPerRank*numRanks]);
    std::unique_ptr<bool[]> inLayerSendBuf(new bool[membersPerRank]);
    std::unique_ptr<bool[]> inLayerReceiveBuf(new bool[membersPerRank*numRanks]);

    size_t currentLayer = 1;
    while(potentialLayerMembers.size() != 0){
        PotentialMemberIterator iter = potentialLayerMembers.begin();
        PotentialMemberIterator endIter = potentialLayerMembers.end();
        std::advance(iter, membersPerRank*myRank);
        for(size_t i = 0; i < membersPerRank && iter != endIter; iter++, i++){
            vertIdSendBuf[i] = *iter;
            inLayerSendBuf[i] = allParentsInDepthMap(p, *iter, depthMap);
        }
        
        //Share the information about who's on the current layer
        MPI_Allgather((void*)vertIdSendBuf.get(), membersPerRank,\
                      MPI_INT, (void*)vertIdReceiveBuf.get(), \
                      membersPerRank, MPI_INT, MPI_COMM_WORLD);
        MPI_Allgather((void*)inLayerSendBuf.get(), membersPerRank,\
                      MPI_CXX_BOOL, (void*)inLayerReceiveBuf.get(), \
                      membersPerRank, MPI_CXX_BOOL, MPI_COMM_WORLD);
        
        for(size_t i = 0; i < potentialLayerMembers.size(); i++){
            if(inLayerReceiveBuf[i]){
                VertId completed = vertIdReceiveBuf[i];
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
    SharedVerifier::endClock();

    MPI_Finalize();
    return !result;
}