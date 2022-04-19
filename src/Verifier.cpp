
#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <mpi.h>
#include"Proof.hpp"
#define ROOT_RANK 0
#define CLOCK_RATE 512000000
#define MIN(a,b) (((a)<(b))?(a):(b))

// MPI parameters
int myrank;
int numranks;

// Type Aliases
using Markings = std::unordered_map<int, std::unordered_set<int>>;

void SR(int rank, int vertex_id) {
    std::cout << "Rank " << rank << " was assigned " << vertex_id << std::endl;
}

bool verify(Proof& p) {
    std::vector<int> unallocated_ranks;
    for (int i = 1; i < numranks; i++) {
        unallocated_ranks.push_back(i);
    }

    // Allocate ranks to vertices
    for (const int i: p.assumptions) {
        if (!unallocated_ranks.empty()) {
            const int rank = unallocated_ranks.back();
            unallocated_ranks.pop_back();
            SR(rank, i);
        }
        // SR()
    }

    return false;
}

// Update dest's marking vector to contain source
void mark(Markings& markings, int source, int dest) {
    Markings::iterator it = markings.find(dest);
    if (it == markings.end()) {
        markings[dest] = {};
    }
    markings.at(dest).insert(source);
}

bool hasCompleteMarkings(Proof& p, int vertex_id, const std::unordered_set<int>& markingList) {
    std::unordered_map<HyperslateJustification, int> numMarkingsMap = {
        {Assume, 0}, {AndIntro, 2}, {AndElim, 1},
        {OrIntro, 1}, {OrElim, 3}, {NotIntro, 2},
        {NotElim, 2}, {IfIntro, 1}, {IfElim, 2},
        {IffIntro, 2}, {IffElim, 2}
    };
    // TODO: Maybe error handling
    int markingsNeeded = numMarkingsMap[p.nodeLookup.at(vertex_id).justification];
    return markingList.size() == markingsNeeded;
}

bool verifyVertex(Proof& p, int vertex_id) {
    ProofNode& pn = p.nodeLookup.at(vertex_id);
    switch (pn.justification) {
        case Assume:
            break;
        case AndIntro:
            break;
        case AndElim:
            break;
        case OrIntro:
            break;
        case OrElim:
            break;
        case NotIntro:
            break;
        case NotElim:
            break;
        case IfIntro:
            break;
        case IfElim:
            break;
        case IffIntro:
            break;
        case IffElim:
            break;
        default: break;
    }
    return false;
}

bool verifySimple(Proof& p) {
    Markings markings;
    int numVerified = 0;
    std::queue<int> lastVerified;

    // Assumptions are verified by default
    for (const int vertex_id: p.assumptions) {
        numVerified += 1;
        lastVerified.push(vertex_id);
    }

    while (!lastVerified.empty()) {
        const int vertex_id = lastVerified.front();
        lastVerified.pop();

        // Mark all of a vertex's children
        const ProofNode n = p.nodeLookup.at(vertex_id);
        for (const int child_id: n.children) {
            mark(markings, vertex_id, child_id);
        }

        // Check to see if any markings are completed
        for (auto const& markingIt : markings) {
            const int vid = markingIt.first;
            const std::unordered_set<int>& markingList = markingIt.second;
            if (hasCompleteMarkings(p, vid, markingList)) {
                if (verifyVertex(p, vid)) {
                    lastVerified.push(vid);
                    markings.erase(vid);
                    // TODO: Update assumptions
                } else {
                    // One false vertex means its all false
                    return false;
                }
            }
        }
    }

    // If the number verified matches the graph, then the
    // whole graph was verified successfully.
    return numVerified == p.nodeLookup.size();
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
        bool verification_result_simple = verifySimple(p);
        bool verification_result_multi = verification_result_simple;
        if (numranks > 1) {
            bool verification_result_multi = verify(p);
        }
        if (verification_result_simple != verification_result_multi) {
            std::cerr << "Warning: Single and Multiprocess Implemenations Don't Match!" << std::endl;
        } else {
            if (verification_result_multi) {
                std::cout << "Verification Successful." << std::endl;
            } else {
                std::cout << "Verification Failed." << std::endl;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}