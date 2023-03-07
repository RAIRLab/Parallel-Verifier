
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

//Depthmaps map each node in the proof graph to a layer number
//representing the maximum distance to an assumption
using DepthMap = std::unordered_map<VertId, size_t>;

//Layer Map takes a layer and returns a list of nodes on that layer.  
using LayerMap = std::unordered_map<size_t, std::unordered_set<VertId>>;

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


//Takes a proof and returns a map from layer numbers to sets of vertices
//on that layer.
//Works by climbing down the proof tree from the assumptions 
//Because it starts at assumptions, the map is not gaurnteed to contain
//every node in P if the node is stand alone without any assumptions as
//a parent, however these cases only occour is a proof is false and
//will not be verified
std::pair<LayerMap, DepthMap> getLayerAndDepthMaps(const Proof& p){
    
    //Maps each node (VertID) to its layer number (size_t)
    DepthMap depthMap; 
    //Maps each layer number (size_t) to the set of nodes
    //(std::unordered_set<VertId>) on that layer
    LayerMap layerMap; 

    //Stores the nodes that are posibly members of the current layer
    std::unordered_set<VertId> potentialLayerMembers;

    //Add all assumptions to the depth map at level 0
    //and their children as possible members of layer 1
    layerMap.insert({0, std::unordered_set<VertId>()});
    for(const VertId assumption : p.assumptions){
        depthMap.insert({assumption, 0});
        layerMap[0].insert(assumption);
        for(VertId assumptionChild : p.nodeLookup.at(assumption).children){
            potentialLayerMembers.insert(assumptionChild);
        }
    }

    //While there are nodes that could potenially be members of the current layer
    //(stored within potentialLayerMembers) split them up among each rank
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
        //uint_8t used as a standin for bool so it plays nice
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
        layerMap.insert({currentLayer, std::unordered_set<VertId>()});
        for(int i = 0; i < numItems; i++){
            VertId completed = potentialMembersCopy[i];
            if(inLayerReceiveBuf[i]){
                depthMap.insert({completed, currentLayer});
                layerMap[currentLayer].insert(completed);
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

bool verifyParallel(const Proof& p){
    auto [layerMap, depthMap] = getLayerAndDepthMaps(p);
    if(depthMap.size() != p.nodeLookup.size())
        return false;

    if(myRank == 0){
        for(const auto [layer, set] : layerMap){
            std::cout<<"Layer "<<layer<<":";
            for(const auto id : set){
                std::cout<<id<<",";
            }
            std::cout<<"\b"<<std::endl;
        }
    }
    
    return true;
}

std::string getFileContentsMPI(const char* filePath){
    MPI_File fileHandle;
    MPI_File_open(MPI_COMM_WORLD, filePath, MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &fileHandle);
    MPI_Offset fileSize;
    MPI_File_get_size(fileHandle, &fileSize);
    char* fileContentsBuff = new char[fileSize];
    MPI_File_read(fileHandle, fileContentsBuff, fileSize,
                  MPI_CHAR, MPI_STATUS_IGNORE);
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