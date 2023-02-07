
#include "ProofIO.hpp"
#include <iostream>
#include <cstdio>
#include <string>
#include <filesystem>

using namespace ProofIO;

ProofData::ProofData()
:tag(ProofData::Tag::Lazyslate), lazyslateData() {
} 

ProofData::~ProofData(){
    //This was only required when we had it as an anonymous union
    /*
    switch(this->tag) {
        case ProofData::Tag::Hyperslate:
            hyperslateData.~FileData();
            break;
        case ProofData::Tag::Lazyslate:
            lazyslateData.~FileData();
            break;
        default:
            std::cerr<<"ProofIO Error: could not destruct "\
            "ProofData object, invalid proof type";
    }
    */
}

ProofData::ProofData(const ProofData& copy) {
    this->tag = copy.tag;
    switch(copy.tag) {
        case ProofData::Tag::Hyperslate:
            this->hyperslateData = copy.hyperslateData;
            break;
        case ProofData::Tag::Lazyslate:
            this->lazyslateData = copy.lazyslateData;
            break;
        default:
            throw std::runtime_error("ProofIO Error: could not construct "\
            "ProofData object, invalid proof type");
    }
}

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

ProofData ProofIO::loadData(std::string filename) {
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
        returnData.tag = ProofData::Tag::Hyperslate;
        returnData.hyperslateData = hyperslate::parse(fileContents);
    } else if (fileExtension == ".json") {
        returnData.tag = ProofData::Tag::Lazyslate;
        returnData.lazyslateData = lazyslate::parse(fileContents);
    }else{
        throw std::runtime_error("ProofIO error: Unsupported File Type \"" 
                                  + fileExtension + "\"");
    }
    return returnData;
}


Proof ProofIO::loadProof(std::string filename) {
    ProofData data = loadData(filename);
    switch (data.tag) {
        case ProofData::Tag::Hyperslate:
            return hyperslate::constructProof(data.hyperslateData);
            break;
        case ProofData::Tag::Lazyslate:
            return lazyslate::constructProof(data.lazyslateData);
            break;
    }
    throw std::runtime_error("ProofIO error: Invalid ProofData Tag");
}
