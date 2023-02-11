
#include<iostream>
#include<cstdio>
#include<filesystem>

namespace fs = std::filesystem;

std::pair<FILE*, FILE*> argParse(int argc, char** argv) {
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

//Takes in a file and returns the contents as a string
//Optimized for speed
std::string readFileContents(FILE* inputFile){
    std::string returnString;
    std::fseek(inputFile, 0, SEEK_END);
    size_t fileSize = std::ftell(inputFile);
    returnString.resize(fileSize);
    std::rewind(inputFile);
    fread(&returnString[1], 1, fileSize, inputFile);
    return returnString;
}

int main(int argc, char** argv) {
    const auto [inputFile, outputFile] = argParse(argc, argv);
    std::string inputFileContents = readFileContents(inputFile);
    
}