
#include "ProofIO.hpp"
#include <iostream>
#include <cstdio>
#include <string>
#include <filesystem>

using namespace ProofIO;

//Speed optimized file contents to string
std::string readFileContents(FILE* inputFile){
    std::string returnString;
    std::fseek(inputFile, 0, SEEK_END);
    size_t fileSize = std::ftell(inputFile);
    returnString.resize(fileSize);
    std::rewind(inputFile);
    fread(&returnString[0], 1, fileSize, inputFile);
    return returnString;
}

ProofData ProofIO::loadProofData(std::string filename) {
    FILE* inputFile = std::fopen(filename.c_str(), "r");
    if(!inputFile){
        throw std::runtime_error("ProofIO Error: Unable to open file \"" \
        + filename + "\"");
    }
    std::string fileContents = readFileContents(inputFile);
    
    //Determine the file type based off the extension
    std::filesystem::path filePath(filename);
    std::string fileExtension = filePath.extension();
    ProofData returnData;
    if (fileExtension == ".slt") {
        returnData = hyperslate::parse(fileContents);
    } else if (fileExtension == ".json") {
        returnData = lazyslate::parse(fileContents);
    }else{
        throw std::runtime_error("ProofIO error: Unsupported File Type \"" 
                                  + fileExtension + "\"");
    }
    return returnData;
}


Proof ProofIO::loadProofFromFile(std::string filename) {
    ProofData data = loadProofData(filename);
    switch (data.index()) {
        case 0: //Hyperslate
            return hyperslate::constructProof(std::get<0>(data));
            break;
        case 1: //Lazyslate
            return lazyslate::constructProof(std::get<1>(data));
            break;
    }
    throw std::runtime_error("ProofIO error: Invalid ProofData Tag");
}

Proof ProofIO::loadFromJSONContents(const std::string& fileContents){
    lazyslate::FileData data = lazyslate::parse(fileContents);
    return lazyslate::constructProof(data);
}