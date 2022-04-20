
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
using Markings = std::unordered_map<id_t, std::unordered_set<id_t>>;
using Assumptions = std::unordered_map<id_t, std::unordered_set<id_t>>;

void SR(int rank, int vertex_id) {
    std::cout << "Rank " << rank << " was assigned " << vertex_id << std::endl;
}

bool verify(Proof& p) {
    std::vector<int> unallocated_ranks;
    for (int i = 1; i < numranks; i++) {
        unallocated_ranks.push_back(i);
    }

    // Allocate ranks to vertices
    for (id_t i: p.assumptions) {
        if (!unallocated_ranks.empty()) {
            int rank = unallocated_ranks.back();
            unallocated_ranks.pop_back();
            SR(rank, i);
        }
        // SR()
    }

    return false;
}

// Update dest's marking vector to contain source
void mark(Markings& markings, id_t source, id_t dest) {
    Markings::iterator it = markings.find(dest);
    if (it == markings.end()) {
        markings.insert({dest, std::unordered_set<id_t>()});
        // markings[dest] = std::unordered_set<id_t>();
    }
    markings.at(dest).insert(source);
}

bool hasCompleteMarkings(const Proof& p, id_t vertex_id, const std::unordered_set<id_t>& markingList) {
    std::unordered_map<HyperslateJustification, size_t> numMarkingsMap = {
        {Assume, 0}, {AndIntro, 2}, {AndElim, 1},
        {OrIntro, 1}, {OrElim, 3}, {NotIntro, 2},
        {NotElim, 2}, {IfIntro, 1}, {IfElim, 2},
        {IffIntro, 2}, {IffElim, 2}
    };
    size_t markingsNeeded = numMarkingsMap[p.nodeLookup.at(vertex_id).justification];
    return markingList.size() == markingsNeeded;
}

inline bool is_and_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "and";
}

bool verifyAndIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_and_vertex(pn)) {
        return false;
    }

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    // Make sure the subformulas match the parents
    bool match_parent1 = false;
    bool match_parent2 = false;
    id_t parent1Id;
    id_t parent2Id;
    for (id_t parent_id : pn.parents) {
        const ProofNode& parent_pn = p.nodeLookup.at(parent_id);
        if (parent_pn.formula == pn.formula.members[1]) {
            match_parent1 = true;
            parent1Id = parent_id;
        } else if (parent_pn.formula == pn.formula.members[2]) {
            match_parent2 = true;
            parent2Id = parent_id;
        }
    }

    const bool result = match_parent1 && match_parent2;

    // Update assumptions with union
    if (result) {
        assumptions[vertex_id] = assumptions[parent1Id];
        assumptions[vertex_id].insert(assumptions[parent2Id].begin(), assumptions[parent2Id].end());
    }

    return result;
}

bool verifyAndElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    // Grab parent node
    const id_t parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    if (!is_and_vertex(parent_pn)) {
        return false;
    }

    // Make sure the formula matches one of the parent subformulas
    const sExpression& parent_subformula1 = parent_pn.formula.members[1];
    const sExpression& parent_subformula2 = parent_pn.formula.members[2];

    const bool result = pn.formula == parent_subformula1 || pn.formula == parent_subformula2;

    // Update assumptions with union
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result;
}

inline bool is_or_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 3 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "or";
}

bool verifyOrIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_or_vertex(pn)) {
        return false;
    }

    // Make sure we have one parent nodes
    if (pn.parents.size() != 1) {
        return false;
    }

    // Make sure the subformulas match the parents
    const id_t parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    const bool result = parent_pn.formula == pn.formula.members[1] || parent_pn.formula == pn.formula.members[2];

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parentId];
    }

    return result;
}

inline bool is_if_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "if";
}

bool verifyIfElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    id_t firstParentId;
    id_t secondParentId;

    id_t index = 0;
    for (const id_t parentId : pn.parents) {
        if (index == 0) {
            firstParentId = parentId;
        } else {
            secondParentId = parentId;
        }
        index += 1;
    }

    const ProofNode& firstParent = p.nodeLookup.at(firstParentId);
    const ProofNode& secondParent = p.nodeLookup.at(secondParentId);

    const bool result = \
        // firstParent is the antecedant of secondParent
        (is_if_vertex(secondParent) && firstParent.formula == secondParent.formula.members[1]) || \
        // secondParent is the antecedant of firstParent
        (is_if_vertex(secondParent) && firstParent.formula == secondParent.formula.members[1]);

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[firstParentId];
        assumptions[vertex_id].insert(assumptions[secondParentId].begin(), assumptions[secondParentId].end());
    }

    return result;
}

inline bool is_iff_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "iff";
}

bool verifyIffElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    id_t firstParentId;
    id_t secondParentId;

    id_t index = 0;
    for (const id_t parentId : pn.parents) {
        if (index == 0) {
            firstParentId = parentId;
        } else {
            secondParentId = parentId;
        }
        index += 1;
    }

    const ProofNode& firstParent = p.nodeLookup.at(firstParentId);
    const ProofNode& secondParent = p.nodeLookup.at(secondParentId);

    const bool result =  \
        // firstParent is the antecedant of secondParent
        (is_if_vertex(secondParent) && \
            (firstParent.formula == secondParent.formula.members[1] || \
            firstParent.formula == secondParent.formula.members[2])
        ) || \
        // secondParent is the antecedant of firstParent
        (is_if_vertex(secondParent) && \
            (firstParent.formula == secondParent.formula.members[1] || \
            firstParent.formula == secondParent.formula.members[2])
        );

    if (result) {
        assumptions[vertex_id] = assumptions[firstParentId];
        assumptions[vertex_id].insert(assumptions[secondParentId].begin(), assumptions[secondParentId].end());
    }

    return result;
}

// Verify that vertex is justified and update assumptions
bool verifyVertex(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);
    switch (pn.justification) {
        case Assume:
            return true;
        case AndIntro:
            return verifyAndIntro(p, vertex_id, assumptions);
        case AndElim:
            return verifyAndElim(p, vertex_id, assumptions);
        case OrIntro:
            return verifyOrIntro(p, vertex_id, assumptions);
        case OrElim:
            return true;
        case NotIntro:
            return true;
        case NotElim:
            return true;
        case IfIntro:
            return true;
        case IfElim:
            return verifyIfElim(p, vertex_id, assumptions);
        case IffIntro:
            return true;
        case IffElim:
            return verifyIffElim(p, vertex_id, assumptions);
        default: break;
    }
    return false;
}

bool verifySimple(Proof& p) {
    Markings markings;
    Assumptions assumptions;
    int numVerified = 0;
    std::queue<id_t> lastVerified;

    // Assumptions are verified by default
    for (id_t vertex_id: p.assumptions) {
        numVerified += 1;
        lastVerified.push(vertex_id);
        std::cout << "Verified " << vertex_id << std::endl;
    }

    while (!lastVerified.empty()) {
        const id_t vertex_id = lastVerified.front();
        lastVerified.pop();

        // Mark all of a vertex's children
        const ProofNode n = p.nodeLookup.at(vertex_id);
        for (const id_t child_id: n.children) {
            mark(markings, vertex_id, child_id);
        }

        // Check to see if any markings are completed
        std::vector<id_t> toEraseMarking;
        for (auto const& markingIt : markings) {
            id_t vid = markingIt.first;
            const std::unordered_set<id_t>& markingList = markingIt.second;
            if (hasCompleteMarkings(p, vid, markingList)) {
                // If marking completed, verify the vertex
                if (verifyVertex(p, vid, assumptions)) {
                    // Verified vertices are the new starting points
                    // and have their markings removed.
                    lastVerified.push(vid);
                    toEraseMarking.push_back(vid);
                    std::cout << "Verified " << vid << std::endl;
                    numVerified += 1;
                } else {
                    // One false vertex means its all false
                    return false;
                }
            }
        }
        // Erased verified vertices in the markings map
        for (id_t id : toEraseMarking) {
            markings.erase(id);
        }
    }

    // If the number verified matches the graph node size,
    // then the whole graph was verified successfully.
    // std::cout << "Verified " << numVerified << "/" << p.nodeLookup.size() - 1 << std::endl;
    // Minus 1 since there's a node labeled ":"
    return numVerified == p.nodeLookup.size() - 1;
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
