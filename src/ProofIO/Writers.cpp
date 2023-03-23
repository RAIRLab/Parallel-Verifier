//For writing proofs in various formats

#include<unordered_map>
#include<random>
#include<cassert>
#include<cstdio>

#include "ProofIO.hpp"

using Vec2 = std::pair<double, double>;
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

Vec2 distance(const Vec2& p1, const Vec2& p2){
    double dx = p1.first - p2.first;
    double dy = p1.second - p2.second;
    return std::make_pair(dx, dy);
}

//SPRING graph construction from
//https://cs.brown.edu/people/rtamassi/gdhandbook/chapters/force-directed.pdf
Vert2Pos spring(const Proof& p){
    //Params for SPRING from

    const unsigned int maxIters = 100;
    const double c1 = 2, c2=1, c3 = 1, c4 = 0.1;
    const int xBound = 1000, yBound = 1000;
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
            Vec2& p1 = retVals[nodeId1];
            Vec2 forceSum(0, 0);
            for(const auto& [nodeId2, node2] : p.nodeLookup){
                if(nodeId1 == nodeId2){
                    continue;
                }
                Vec2& p2 = retVals[nodeId2];
                Vec2 dist = distance(p1, p2);

                //Attractive force
                double fax = log(dist.first / c2);
                double fay = log(dist.second / c2);
                Vec2 fa(fax, fay);
                
                //repellant force
                double frx = c3 / (dist.first * dist.first);
                double fry = c3 / (dist.second * dist.second);
                forceSum.first += fax + frx;
                forceSum.second += fay + fry;
            }
            double newPosX = c4 * (p1.first + forceSum.first);
            double newPosY = c4 * (p1.second + forceSum.second);
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
    jsonContents += "], \"links\":[";
    for(const auto& [nodeId, node] : proof.nodeLookup){
        for(const VertId childId : node.children){
            jsonContents += "[" + \
            std::to_string(node.id) + "," + \
            std::to_string(proof.nodeLookup.at(childId).id) + "],";
        }
    }
    jsonContents += "]}";
    jsonContents.reserve();
    writeFileContents(filename, jsonContents);  
}