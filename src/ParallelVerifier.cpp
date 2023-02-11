
#include "ProofIO/Proof.hpp"
#include "SharedVerifier/SharedVerifier.hpp"

int main(int argc, char** argv){
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof p(proofFilePath);
    return 0;
}