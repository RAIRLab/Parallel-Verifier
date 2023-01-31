
#include "ProofIO.hpp"
#include <cstdio>
#include <string>

using namespace ProofIO;

//Speed optimized file contents to string
std::string readFileContents(FILE* inputFile){
    std::string returnString;
    std::fseek(inputFile, 0, SEEK_END);
    size_t fileSize = std::ftell(inputFile);
    returnString.resize(fileSize);
    std::rewind(inputFile);
    fread(&returnString[1], 1, fileSize, inputFile);
    return returnString;
}

ProofData loadData(std::string filename) {

}


Proof loadProof(std::string filename) {
    ProofData data = loadData(filename);
    switch (data.tag) {
        case ProofData::Tag::Hyperslate:
            return hyperslate::
            break;
        
        default:
            break;
    }
}
