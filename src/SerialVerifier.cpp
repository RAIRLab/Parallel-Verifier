
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include"SharedVerifier.hpp"

bool verifySimple(Proof& p) {
    Markings markings;
    Assumptions assumptions;
    int numVerified = 0;
    std::queue<vertId> lastVerified;

    // Assumptions are verified by default
    for (vertId vertex_id: p.assumptions) {
        verifyVertex(p, vertex_id, assumptions);
        numVerified += 1;
        lastVerified.push(vertex_id);
        // std::cout << "Verified " << vertex_id << std::endl;
    }

    while (!lastVerified.empty()) {
        const vertId vertex_id = lastVerified.front();
        lastVerified.pop();

        // Mark all of a vertex's children
        const ProofNode n = p.nodeLookup.at(vertex_id);
        for (const vertId child_id: n.children) {
            mark(markings, vertex_id, child_id);
        }

        // Check to see if any markings are completed
        std::vector<vertId> toEraseMarking;
        for (auto const& markingIt : markings) {
            vertId vid = markingIt.first;
            const std::unordered_set<vertId>& markingList = markingIt.second;
            if (hasCompleteMarkings(p, vid, markingList)) {
                // If marking completed, verify the vertex
                if (verifyVertex(p, vid, assumptions)) {
                    // Verified vertices are the new starting points
                    // and have their markings removed.
                    lastVerified.push(vid);
                    toEraseMarking.push_back(vid);
                    // std::cout << "Verified " << vid << std::endl;
                    numVerified += 1;
                } else {
                    // One false vertex means its all false
                    return false;
                }
            }
        }
        // Erased verified vertices in the markings map
        for (vertId id : toEraseMarking) {
            markings.erase(id);
        }
    }

    // If the number verified matches the graph node size,
    // then the whole graph was verified successfully.
    // std::cout << "Verified " << numVerified << "/" << p.nodeLookup.size() << std::endl;
    return numVerified == p.nodeLookup.size();
}

int main(int argc, char** argv){
    const char* proofFilePath = VerifierInit(argc, argv);
    std::ifstream fileStream(proofFilePath);
    std::string fileConents((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    Proof proof(fileConents);
    bool result = verifySimple(proof);
    std::cout<<result<<std::endl;
    return 0;
}