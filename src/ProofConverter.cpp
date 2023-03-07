
#include<iostream>
#include<cstdio>
#include<filesystem>

#include "Proof/Proof.hpp"

namespace fs = std::filesystem;

std::pair<Proof,FILE*> argParse(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: ProofConverter [outputFile.json] inputFile.slt"
        << std::endl;
        exit(1);
    }

    fs::path inputFilePath;
    fs::path outputFilePath(argv[1]);
    if (argc == 2) {
        outputFilePath.replace_extension(".json");
        inputFilePath = fs::path(argv[1]);
    } else {
        inputFilePath = fs::path(argv[2]);
    }

    FILE* inputFile = std::fopen(inputFilePath.c_str(), "r");
    if (!inputFile) {
        std::cerr << "Failed To Open " << inputFilePath << std::endl;
    }
    FILE* outputFile = std::fopen(outputFilePath.c_str(), "r");
    if (!outputFile) {
        std::cerr << "Failed To Open " << outputFilePath << std::endl;
    }
    return std::make_pair(inputFile, outputFile);
}

int main(int argc, char** argv){

}