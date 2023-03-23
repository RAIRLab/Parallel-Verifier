//For writing proofs in various formats

#include<unordered_map>
#include<random>
#include<cassert>
#include<cstdio>

#include "ProofIO.hpp"

using Vec2 = std::pair<int, int>;
using Vert2Pos = std::unordered_map<VertId, Vec2>;


//Speed optimized file contents to string
void writeFileContents(std::string outFilePath, const std::string& contents){
    FILE* file = std::fopen(outFilePath.c_str(), "w");
    if(!file){
        throw std::runtime_error("Could not open file for writing: " + \
                                  outFilePath);
    }
    size_t sizeWritten = \
        std::fwrite(contents.c_str(), sizeof(char), contents.length(), file);
    assert(sizeWritten == contents.length());
}

int randBetween(int low, int high){
    assert(low < high);
    return rand() % (high - low + 1) + low;
}

//SPRING graph construction from
//https://cs.brown.edu/people/rtamassi/gdhandbook/chapters/force-directed.pdf
Vert2Pos spring(const Proof& p){
    //Params for SPRING from

    const unsigned int maxIters = 100;
    const double c1 = 10, c2=1, c3 = 100, c4 = 0.5;
    const int xBound = 400, yBound = 400;
    Vert2Pos retVals;

    //Place nodes in random positions on the screen
    
    for(const auto [nodeId, node] : p.nodeLookup){
        int randX = randBetween(-xBound, xBound);
        int randY = randBetween(-yBound, yBound);
        retVals[nodeId] = std::pair(randX, randY);
    }

    Vert2Pos newVals;
    for(unsigned int iters = 0; iters < maxIters; iters++){
        for(const auto& [nodeId1, node1] : p.nodeLookup){
            const Vec2& p1 = retVals.at(nodeId1);
            double fSumX = 0;
            double fSumY = 0;
            for(const auto& [nodeId2, node2] : p.nodeLookup){
                if(nodeId1 == nodeId2){
                    continue;
                }
                Vec2& p2 = retVals[nodeId2];
                double dx = abs(p1.first - p2.first);
                double dy = abs(p1.second - p2.second);

                //Attractive force
                dx = dx < 0.1 ? 0.1: dx;
                dy = dy < 0.1 ? 0.1: dy;
                double fax = c1 * log(dx / c2);
                double fay = c1 * log(dy / c2);

                //repellant force
                double frx = c3 / (dx * dx);
                double fry = c3 / (dy * dy);
                fSumX += fax + frx;
                fSumY += fay + fry;
            }
            double newPosX = c4 * (p1.first + fSumX);
            double newPosY = c4 * (p1.second + fSumY);
            newVals[nodeId1] = Vec2(newPosX, newPosY);
        }
        retVals = newVals;
    }
    return retVals;
}

void ProofIO::toLazyslateFile(std::string filename, const Proof& proof){
    auto rjm = lazyslate::reverseJustificationMap;
    Vert2Pos positions = spring(proof);
    
    std::string jsonContents;
    //Minimally reserve 200 characters per node to speed up all these
    //string concatenations
    jsonContents.reserve(200 * proof.nodeLookup.size());
    jsonContents += "{\"nodes\":[";
    for(const auto& [nodeId1, node] : proof.nodeLookup){
        jsonContents += std::string("{") + \
        "\"id\":" + std::to_string(node.id) + "," + \
        "\"name\":\"" + std::to_string(node.id) + "\"," + \
        "\"expression\":\"" + node.formula.toString() + "\"," + \
        "\"justification\":\"" + rjm[node.justification] + "\"," + \
        "\"position\":{" + \
            "\"x\":" + std::to_string(positions[nodeId1].first) + "," + \
            "\"y\":" + std::to_string(positions[nodeId1].second) + \
        "}},"; 
    }
    jsonContents.pop_back();
    jsonContents += "], \"links\":[";
    for(const auto& [nodeId, node] : proof.nodeLookup){
        for(const VertId childId : node.children){
            jsonContents += "[" + \
            std::to_string(node.id) + "," + \
            std::to_string(proof.nodeLookup.at(childId).id) + "],";
        }
    }
    jsonContents.pop_back();
    jsonContents += "]}";
    jsonContents.reserve();
    writeFileContents(filename, jsonContents);  
}