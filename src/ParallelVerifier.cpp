
#include<mpi.h>
#include<functional>
#include<iostream>
#include"SharedVerifier.hpp"
#include"timing.hpp"

//MPI globals
int myRank;
int numRanks;

/*
Marking Matrices are how we pass markings and assumptions to other MPI ranks.
(The name "marking matrix" is a holdover from before I used them for assumptions)
Since in our model, one rank can own multiple vertices who can potentially mark at the same time,
we need to be able to pass markings from multiple verticies in a singe MPI structure.
Sample marking matrix:
    1  2  3  -1   <--- Special header row specifies which vertex (id) is doing the marking
    6  6  8  -1         (for assumptions it indicates which vertex has these assumptions)
    7  -1 -1 -1  <-- -1 in a row with a header node means nothing else is being marked by this vertex
              ^ -1 in the header means col unused, this is necessary because MPI 
                forces marking Matrices to be the same size for MPI_Allgather
                (The global agreed upon size must be computed via MPI_Allreduce beforehand)
    This marking matrix can be read as: 
    "vertex 1 marks 6 and 7, vertex 2 marks 6, vertex 3 marks 8"
    Alterntively if interpreted as an assumption matrix:
    "vertex 1 assumes 6 and 7, vertex 2 assumes 6, vertex 3 assumes 8"
*/
using MarkingMatrix = std::vector<std::vector<vertId>>;

//This serializes a marking matrix into a list of ints for MPI_Allgather
inline std::vector<int> flattenMarkingMatrix(const MarkingMatrix& matrix){
    size_t len = matrix.size() == 0 ? 0 : matrix.size()*matrix[0].size();
    std::vector<int> returnVec(len);
    for(int i = 0; i < matrix.size(); i++)
        for(int j = 0; j < matrix[0].size(); j++)
            returnVec[i*matrix[0].size()+j] = matrix[i][j];
    return returnVec;
}

//Pad out a marking matrix with -1s after the with and height
//for the Allgather have been determined via MPI_allreduce
void padMarkingMatrix(MarkingMatrix& mm, int mmWidth, int mmHeight){
    for(int i = 0; i < mm.size(); i++)
        for(int j = mm[i].size(); j < mmHeight; j++)
            mm[i].push_back(-1);
    for(int i = mm.size(); i < mmWidth; i++){
        mm.push_back(std::vector<vertId>(mmHeight, -1));
    }
}

/*
    the marking cube is a stack of marking matrixs created by MPI_Allgather
    the outrmost dimention is the rank.
*/
using MarkingCube = std::vector<MarkingMatrix>;

//Ok this is kind of slow because I copy a bunch of matricies 
MarkingCube makeMarkingCube(const std::vector<int>& allgathered, int cols, int rows){
    MarkingCube returnCube = std::vector<std::vector<std::vector<int>>>(numRanks, std::vector<std::vector<int>>(cols, std::vector<int>(rows, -1)));
    for(int i = 0; i < numRanks; i++)
        for(int j = 0; j < cols; j++)
            for(int k = 0; k < rows; k++)
                returnCube[i][j][k] = allgathered[i*cols*rows+j*rows+k];
    return returnCube;
}

//Old print function for debugging MPI
/*void printMarkings(Markings m){
    for (const std::pair<vertId, std::unordered_set<vertId>>& ms : m){
        std::cout<<ms.first<<":";
        for(const vertId mkd : ms.second)
            std::cout<<mkd<<" ";
        std::cout<<"\n";
    }
    std::cout<<"=====^"<<myRank<<"==============="<<std::endl;
}*/

//Handles updating the shared states (Markings and Assumptions) when provided with the newly calculated
//Markings or assumptions in the form of a jaggad MarkingMatrix mm. The mmWidthLocal and mmHeightLocal
//Should be the max width and height of a row/col in mm.
//preforms all MPI message formatting and provides a callback to update Markings and Assumptions
void updateAttribute(MarkingMatrix& mm, int mmWidthLocal, int mmHeightLocal, Markings& markings, std::function<void(vertId, vertId)> updateCallback){
    //serialize the marking for MPI
    int mmWidth, mmHeight;
    MPI_Allreduce(&mmWidthLocal, &mmWidth, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&mmHeightLocal, &mmHeight, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    padMarkingMatrix(mm, mmWidth, mmHeight);
    std::vector<int> serializedMarkings = flattenMarkingMatrix(mm);

    //Obtain markings the markings
    std::vector<int> serializedMarkingsCube(mmWidth*mmHeight*numRanks);
    MPI_Allgather(serializedMarkings.data(), serializedMarkings.size(), MPI_INT,
                    serializedMarkingsCube.data(), serializedMarkings.size(), MPI_INT,
                    MPI_COMM_WORLD);

    MarkingCube markingCube = makeMarkingCube(serializedMarkingsCube, mmWidth, mmHeight);

    //Update our markings
    
    for(int rank = 0; rank < markingCube.size(); rank++){
        for(int col = 0; col < markingCube[0].size(); col++){
            //The vertex that did the marking
            vertId marker = markingCube[rank][col][0];
            if(marker == -1)
                break;
            for(int row = 1; row < markingCube[0][0].size(); row++){
                vertId markee = markingCube[rank][col][row];
                if(markee == -1)
                    break;
                updateCallback(marker, markee);
            }
        }
    }
}



bool verify(Proof proof){
    //keeps track of assumptions and markings globally for load distribution
    Assumptions assumptions;
    Markings markings;

    std::unordered_set<vertId> vertices = proof.assumptions; //all verticies in play

    while(true){

        /*for(auto c : vertices)
            std::cout << c << ' ';
        std::cout<<"<-----"<<myRank<<"-----"<<std::endl;*/

        /* Old TODO
            James for the love of god remember to come back and make it so we're not distributing round robin
            on any verts that don't already hasCompleteMarkings. Do the hasCompleteMarkings check at the end
            of the algo and change verticies to just be the set of verticies that actually needs to be verified
        */

        //New TODO this is still probably sub-optimal, ideally this can be computed at the end instead of placing into verts
        std::unordered_set<vertId> completeVerts;
        for(vertId vertexId : vertices)
            if(hasCompleteMarkings(proof, vertexId, markings[vertexId]))
                completeVerts.insert(vertexId);
        

        //Round robin distribute ownwership of all verts in play to all ranks,
        // one potential optimization would be doing this distribution
        // with respect to a heuristic rather than round robin.
        std::unordered_set<vertId> owned; //verticies this rank "owns" (will attempt to verify) for this iteration 
        std::vector<vertId> allVerts(completeVerts.begin(), completeVerts.end());
        for(int i = myRank; i < allVerts.size(); i += numRanks)
            owned.insert(allVerts[i]);
        
        MarkingMatrix markingMatrix;
        MarkingMatrix assumptionMatrix;
        bool updateLocal = false;        //If no marks were placed, end the program and return true;
        bool failedLocal = false;        //If a vertexVerify failed, end the program and return false;
        int numMarkingOwners = 0;   //number of verticies this rank owns that will mark this iteration
        int maxNumChildren = 0;     //max number of children to mark
        int maxAssumptions = 0;
        for(const vertId ownerId : owned){
            //if(hasCompleteMarkings(proof, ownerId, markings[ownerId])){
                Assumptions passumptions = assumptions;
                bool verified = verifyVertex(proof, ownerId, assumptions);
                /*if(!verified){
                    std::cout<<"Wrong! : "<< ownerId << " " << proof.nodeLookup[ownerId].formula.toString()<<"\n";
                    printMarkings(passumptions);
                    std::cout<<"new assumptions! :\n";
                    printMarkings(assumptions);
                    std::cout<<"markings :\n";
                    printMarkings(markings);
                }*/

                failedLocal = failedLocal || !verified;
                std::vector<vertId> curMarkings;
                std::vector<vertId> curAssumptions;
                //Marking Message generation code
                curMarkings.push_back(ownerId); //The header contains the marking node
                curAssumptions.push_back(ownerId); //The header containing the node these are assumptions for
                //The rest of the col contains the markings
                for(const vertId childId : proof.nodeLookup[ownerId].children){
                    curMarkings.push_back(childId);
                    updateLocal = true; //We wont end if there is a marking update to do
                }
                for(const vertId assumptionId : assumptions[ownerId])
                    curAssumptions.push_back(assumptionId);
                markingMatrix.push_back(curMarkings);
                assumptionMatrix.push_back(curAssumptions);
                maxNumChildren = std::max(maxNumChildren, (int)curMarkings.size());
                maxAssumptions = std::max(maxAssumptions, (int)curAssumptions.size());
                numMarkingOwners++;
            //}
        }

        //check end conditions
        //check that no ranks have failed to verify, if they have end the function and return false
        bool failed;
        MPI_Allreduce(&failedLocal, &failed, 1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD);
        if(failed)
            return false;
        bool update;
        MPI_Allreduce(&updateLocal, &update, 1, MPI_CXX_BOOL, MPI_LOR, MPI_COMM_WORLD);
        if(!update)
            return true;

        //Share the assumptions
        updateAttribute(assumptionMatrix, numMarkingOwners, maxAssumptions, assumptions,
            [&assumptions](vertId vert, vertId assumptionVert){
                //assumptions map is "vert with assumptions" : "list of assumptions"
                assumptions[vert].insert(assumptionVert);
            });

        //Share the markings
        Markings newMarkings;
        updateAttribute(markingMatrix, numMarkingOwners, maxNumChildren, markings,
            [&markings, &newMarkings](vertId marker, vertId markee){
                //markings map is "vert being marked" : "list of marking verts"
                markings[markee].insert(marker);
                newMarkings[markee].insert(marker);
            });

        //printMarkings(markings);

        //Update verts in play w.r.t the markings that have just been placed
        for (const std::pair<vertId, std::unordered_set<vertId>>& marks : newMarkings){
            vertices.insert(marks.first);
            for(const vertId marked : marks.second)
                vertices.erase(marked);
        }
    }
}

int main(int argc, char** argv){
    //Init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    const char* proofFilePath = VerifierInit(argc, argv);

    //Get the proof via MPI File IO
    MPI_File fileHandle;
    MPI_File_open(MPI_COMM_WORLD, proofFilePath, MPI_MODE_RDONLY, MPI_INFO_NULL, &fileHandle);
    MPI_Offset fileSize;
    MPI_File_get_size(fileHandle, &fileSize);
    char* fileContentsBuff = new char[fileSize];
    MPI_File_read(fileHandle, fileContentsBuff, fileSize, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&fileHandle);
    std::string fileContents = std::string(fileContentsBuff);
    delete[] fileContentsBuff;
    Proof proof(fileContents);

    //Verify
    startClock();
    bool result = verify(proof);
    if(myRank == 0){
        endClock();
        std::cout<<result<<std::endl;
    }

    MPI_Finalize();
    return 0;
}