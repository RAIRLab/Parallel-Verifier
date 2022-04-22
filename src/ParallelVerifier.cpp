

#include<memory>
#include<mpi.h>
#include<iostream>
#include"SharedVerifier.hpp"

int rank;
int numRanks;

//all other ranks
bool verifySlave(Proof p){

}

//rank 0
bool verifyMaster(Proof p){
    
}

//Other ranks
bool verify(Proof p){

}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);
    const char* proofFilePath = VerifierInit(argc, argv);

    MPI_File fileHandle;
    MPI_File_open(MPI_COMM_WORLD, proofFilePath, MPI_MODE_RDONLY, MPI_INFO_NULL, &fileHandle);
    MPI_Offset fileSize;
    MPI_File_get_size(fileHandle, &fileSize);
    std::unique_ptr<char> fileContentsPtr = std::make_unique<char>(fileSize+1);
    MPI_File_read(fileHandle, fileContentsPtr.get(), fileSize, MPI_CHAR, MPI_STATUS_IGNORE);
    MPI_File_close(&fileHandle);
    std::string fileContents = std::string(fileContentsPtr.get());
    Proof proof(fileContents);

    bool result = verify(proof);
    std::cout<<result<<std::endl;

    MPI_Finalize();
    return 0;
}