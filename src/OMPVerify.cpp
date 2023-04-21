

#include "SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier/LibOMPVerifier.hpp"

int main(int argc, char** argv) {
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof proof(proofFilePath);
    
    SharedVerifier::startClock();
    bool result = OMPVerifier::OMPVerify(proof);
    SharedVerifier::endClockPrint();

    return !result;
}