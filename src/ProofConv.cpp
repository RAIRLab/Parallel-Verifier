
#include<cstdio>
#include<cassert>

#include "Proof/Proof.hpp"
#include "ProofIO/ProofIO.hpp"

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr, "Incorrect usage! Use:\n" \
                        "ProofConv [InputFile] [OutputFile]\n");
        exit(1);
    }
    std::string inFileName = argv[1];
    std::string outFileName = argv[2];
    Proof proof(inFileName);
    ProofIO::toLazyslateFile(outFileName, proof);
    return 0;
}