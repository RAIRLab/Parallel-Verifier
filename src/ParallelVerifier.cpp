
#include<cmath>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<list>
#include<numeric>

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

//LayerMaps store sequential sets of verticies
//index 0 will store nodes on layer 0, index 1 will store
//nodes on layer 1, etc.
using LayerMap = std::list<std::unordered_set<VertId>>;


// Debuging Helpers ===========================================================

void MPIDebugPrint(std::string message, int rank = -1){
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == -1 || myRank == rank){
        std::cout<<"Rank "<<myRank<<":"<<message<<std::endl;
    }
}

void PrintLayerMap(const LayerMap& layerMap){
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

// Depth Compuation & associated Helpers ======================================

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
    layerMap.push_back(std::unordered_set<VertId>());
    for(const VertId assumption : p.assumptions){
        depthMap.insert({assumption, 0});
        layerMap.back().insert(assumption);
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
        layerMap.push_back(std::unordered_set<VertId>());
        for(int i = 0; i < numItems; i++){
            VertId completed = potentialMembersCopy[i];
            if(inLayerReceiveBuf[i]){
                depthMap.insert({completed, currentLayer});
                layerMap.back().insert(completed);
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

// Parallel Verification & associated helpers =================================

bool verifyParallel(const Proof& p){
    auto [layerMap, depthMap] = getLayerAndDepthMaps(p);
    
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
        //Example: 5 1 2 3 -1 7 1 3 -1 6 2 -1    N=node A=assuption
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
            std::reduce(assumptionSizes.begin(), assumptionSizes.end());
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