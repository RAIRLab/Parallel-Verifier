
#include<iostream>
#include<string>
#include<mpi.h>
#include"Proof.hpp"
#define ROOT_RANK 0
#define CLOCK_RATE 512000000
#define MIN(a,b) (((a)<(b))?(a):(b))

// MPI parameters
int myrank;
int numranks;

bool verify(Proof p) {
    std::vector<int> unallocated_ranks;
    for (int i = 0; i < numranks; i++) {
        unallocated_ranks.push_back(i);
    }
    return false;
}

void print_help() {
    std::cout << "Usage: ./verif.exe [hyperslate_file]" << std::endl;
}

int main(int argc, char** argv){
    // If no arguments are passed, print help
    if (argc < 2) {
        print_help();
        exit(1);
        return 1;
    }

    // MPI Initialization
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);

    // Parse and create hypergraph on each rank
    std::string filePath(argv[1]);
    HyperslateFileData f = parseHyperslateFile(filePath);
    Proof p(f);

    if (myrank == ROOT_RANK) {
        bool verification_result = verify(p);
        if (verification_result) {
            std::cout << "Verification Successful." << std::endl;
        } else {
            std::cout << "Verification Failed." << std::endl;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}