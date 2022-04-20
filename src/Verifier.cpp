
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
        markings[dest] = std::unordered_set<id_t>();
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

    const bool result = \
        // Check if the formula matches the left parent subformula
        pn.formula == parent_pn.formula.members[1] || \
        // Check if the formula matches the right parent subformula
        pn.formula == parent_pn.formula.members[2];

    // Update assumptions
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

bool verifyOrElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    return true;
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have three parent nodes
    if (pn.parents.size() != 3) {
        return false;
    }

    id_t orParentId;
    id_t parentId2;
    id_t parentId3;
    bool parentId2Found = false;
    bool parentId3Found = false;
    for (const id_t parentId : pn.parents) {
        const ProofNode& parentNode = p.nodeLookup.at(parentId);
        if (parentNode.formula == pn.formula) {
            if (!parentId2Found) {
                parentId2 = parentId;
                parentId2Found = true;
            } else {
                parentId3 = parentId;
                parentId3Found = true;
            }
        } else {
            // Parent vertices that don't match us must be OR rooted.
            if (!is_or_vertex(parentNode)) {
                return false;
            }
            orParentId = parentId;
        }
    }

    // Syntax check: Two of the parents must match the current formula
    if (!parentId2Found || !parentId3Found) {
        return false;
    }

    const ProofNode& orParent = p.nodeLookup.at(orParentId);
    const ProofNode& parentNode2 = p.nodeLookup.at(parentId2);
    const ProofNode& parentNode3 = p.nodeLookup.at(parentId3);

    // Check Assumptions
    bool left_side_check = false;
    id_t parent2AssumptionId;
    bool right_side_check = false;
    id_t parent3AssumptionId;

    for (const id_t a : assumptions[parentId2]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (!left_side_check && aNode.formula == orParent.formula.members[1]) {
            left_side_check = true;
            parent2AssumptionId = a;
            break;
        }
        if (!right_side_check && aNode.formula == orParent.formula.members[2]) {
            right_side_check = true;
            parent2AssumptionId = a;
            break;
        }
    }

    for (const id_t a : assumptions[parentId3]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (!left_side_check && aNode.formula == orParent.formula.members[1]) {
            left_side_check = true;
            parent3AssumptionId = a;
            break;
        }
        if (!right_side_check && aNode.formula == orParent.formula.members[2]) {
            right_side_check = true;
            parent3AssumptionId = a;
            break;
        }
    }


    const bool result = left_side_check && right_side_check;
    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[orParentId];

        std::unordered_set<id_t> parent2Assumptions = assumptions[parentId2];
        parent2Assumptions.erase(parent2AssumptionId);
        std::unordered_set<id_t> parent3Assumptions = assumptions[parentId3];
        parent2Assumptions.erase(parent3AssumptionId);

        assumptions[vertex_id].insert(parent2Assumptions.begin(), parent2Assumptions.end());
        assumptions[vertex_id].insert(parent3Assumptions.begin(), parent3Assumptions.end());
    }

    return result;
}

inline bool is_if_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "if";
}

bool verifyIfIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_if_vertex(pn)) {
        return false;
    }

    // Make sure we only have one parent node
    if (pn.parents.size() != 1) {
        return false;
    }

    const id_t parentId = *pn.parents.begin();
    const ProofNode& parent_pn = p.nodeLookup.at(parentId);

    // Make sure parent is the consequent
    if (parent_pn.formula != pn.formula.members[2]) {
        return false;
    }

    // Make sure the antecedant is in the assumptions of the parent
    bool antecedant_matched = false;
    id_t antecedantId = 0;
    for (const id_t a : assumptions[parentId]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            antecedant_matched = true;
            antecedantId = a;
        }
    }

    // Update Assumptions
    if (antecedant_matched) {
        assumptions[vertex_id] = assumptions[parentId];
        assumptions[vertex_id].erase(antecedantId);
    }

    return antecedant_matched;
}

bool verifyIfElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<id_t> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool result = \
        // firstParent is the antecedant of secondParent
        (is_if_vertex(secondParent) && firstParent.formula == secondParent.formula.members[1]) || \
        // secondParent is the antecedant of firstParent
        (is_if_vertex(secondParent) && firstParent.formula == secondParent.formula.members[1]);

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    }

    return result;
}

inline bool is_iff_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
           pn.formula.members.size() == 3 && \
           pn.formula.members[0].type == sExpression::Type::Symbol && \
           pn.formula.members[0].value == "iff";
}

bool verifyIffIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_iff_vertex(pn)) {
        return false;
    }

    // Make sure we only have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<id_t> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    bool forward_check = false;
    id_t forward_id;
    bool backward_check = false;
    id_t backward_id;

    // Check forward direction
    if (!forward_check && firstParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const id_t a : assumptions[parents[0]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
            }
        }
    } else if (!forward_check && secondParent.formula == pn.formula.members[2]) {
        // Make sure the antecedant is in the assumptions of the parent
        for (const id_t a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                forward_check = true;
                forward_id = a;
            }
        }
    }

    // Check backward direction
    if (!backward_check && firstParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const id_t a : assumptions[parents[0]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                backward_check = true;
                backward_id = a;
            }
        }
    } else if (!backward_check && secondParent.formula == pn.formula.members[1]) {
        // Make sure the consequent is in the assumptions of the parent
        for (const id_t a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                backward_check = true;
                backward_id = a;
            }
        }
    }

    const bool result = forward_check && backward_check;

    // Update Assumptions
    if (result) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(forward_id);
        assumptions[vertex_id].erase(backward_id);
    }

    return result;
}

bool verifyIffElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<id_t> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

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
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
    }

    return result;
}

inline bool is_not_vertex(const ProofNode& pn) {
    return pn.formula.type == sExpression::Type::List && \
       pn.formula.members.size() == 2 && \
       pn.formula.members[0].type == sExpression::Type::Symbol && \
       pn.formula.members[0].value == "not";
}

bool verifyNotIntro(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    if (!is_not_vertex(pn)) {
        return false;
    }

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<id_t> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    bool formula_found = false;
    id_t formula_id = 0;
    // Make sure the current formula is a negation of an assumption
    for (const id_t a : assumptions[parents[0]]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (aNode.formula == pn.formula.members[1]) {
            formula_found = true;
            formula_id = a;
            break;
        }
    }
    if (!formula_found) {
        for (const id_t a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (aNode.formula == pn.formula.members[1]) {
                formula_found = true;
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_found) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id);
    }

    return formula_found;
}

bool verifyNotElim(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);

    // Make sure we have two parent nodes
    if (pn.parents.size() != 2) {
        return false;
    }

    const std::vector<id_t> parents(pn.parents.begin(), pn.parents.end());
    const ProofNode& firstParent = p.nodeLookup.at(parents[0]);
    const ProofNode& secondParent = p.nodeLookup.at(parents[1]);

    const bool syntax_result = \
        // Check to see if the first parent is the neagtion of the second
        (is_not_vertex(firstParent) && firstParent.formula.members[1] == secondParent.formula) || \
        // Check to see if the second parent is the negation of the first
        (is_not_vertex(secondParent) && secondParent.formula.members[1] == firstParent.formula);

    if (!syntax_result) {
        return false;
    }

    bool formula_found = false;
    id_t formula_id = 0;
    // Make sure the current formula is a positive of an assumption
    for (const id_t a : assumptions[parents[0]]) {
        const ProofNode& aNode = p.nodeLookup.at(a);
        if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
            formula_found = true;
            formula_id = a;
            break;
        }
    }
    if (!formula_found) {
        for (const id_t a : assumptions[parents[1]]) {
            const ProofNode& aNode = p.nodeLookup.at(a);
            if (is_not_vertex(aNode) && aNode.formula.members[1] == pn.formula) {
                formula_found = true;
                formula_id = a;
                break;
            }
        }
    }

    // Update Assumptions
    if (formula_found) {
        assumptions[vertex_id] = assumptions[parents[0]];
        assumptions[vertex_id].insert(assumptions[parents[1]].begin(), assumptions[parents[1]].end());
        assumptions[vertex_id].erase(formula_id);
    }

    return formula_found;
}

// Verify that vertex is justified and update assumptions
bool verifyVertex(const Proof& p, id_t vertex_id, Assumptions& assumptions) {
    const ProofNode& pn = p.nodeLookup.at(vertex_id);
    bool result = false;
    switch (pn.justification) {
        case Assume:
            assumptions[vertex_id] = {vertex_id};
            return true;
        case AndIntro:
            result = verifyAndIntro(p, vertex_id, assumptions);
            std::cout << "Passed And Intro: " << result << std::endl;
            break;
            // return verifyAndIntro(p, vertex_id, assumptions);
        case AndElim:
            result = verifyAndElim(p, vertex_id, assumptions);
            std::cout << "Passed And Elim: " << result << std::endl;
            break;
            // return verifyAndElim(p, vertex_id, assumptions);
        case OrIntro:
            result = verifyOrIntro(p, vertex_id, assumptions);
            std::cout << "Passed Or Intro: " << result << std::endl;
            break;
            // return verifyOrIntro(p, vertex_id, assumptions);
        case OrElim:
            result = verifyOrElim(p, vertex_id, assumptions);
            std::cout << "Passed Or Elim: " << result << std::endl;
            break;
            // return verifyOrElim(p, vertex_id, assumptions);
        case NotIntro:
            result = verifyNotIntro(p, vertex_id, assumptions);
            std::cout << "Passed Not Intro: " << result << std::endl;
            break;
            // return verifyNotIntro(p, vertex_id, assumptions);
        case NotElim:
            result = verifyNotElim(p, vertex_id, assumptions);
            std::cout << "Passed Not Elim: " << result << std::endl;
            break;
            // return verifyNotElim(p, vertex_id, assumptions);
        case IfIntro:
            result = verifyIfIntro(p, vertex_id, assumptions);
            std::cout << "Passed If Intro: " << result << std::endl;
            break;
            // return verifyIfIntro(p, vertex_id, assumptions);
        case IfElim:
            result = verifyIfElim(p, vertex_id, assumptions);
            std::cout << "Passed If Elim: " << result << std::endl;
            break;
            // return verifyIfElim(p, vertex_id, assumptions);
        case IffIntro:
            result = verifyIffIntro(p, vertex_id, assumptions);
            std::cout << "Passed Iff Intro: " << result << std::endl;
            break;
            // return verifyIffIntro(p, vertex_id, assumptions);
        case IffElim:
            result = verifyIffElim(p, vertex_id, assumptions);
            std::cout << "Passed Iff Elim: " << result << std::endl;
            break;
            // return verifyIffElim(p, vertex_id, assumptions);
        default:
            std::cout << "Unknown Justification" << std::endl;
            break;
    }
    return result;
}

bool verifySimple(Proof& p) {
    Markings markings;
    Assumptions assumptions;
    int numVerified = 0;
    std::queue<id_t> lastVerified;

    // Assumptions are verified by default
    for (id_t vertex_id: p.assumptions) {
        verifyVertex(p, vertex_id, assumptions);
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
