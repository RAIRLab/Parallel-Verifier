

#include "SharedVerifier/SharedVerifier.hpp"
#include "LibOMPVerifier/LibOMPVerifier.hpp"

int main(int argc, char** argv) {
    const char* proofFilePath = SharedVerifier::init(argc, argv);
    Proof proof(proofFilePath);
    
    SharedVerifier::startClock();
    bool result = OMPVerifier::OMPVerify(proof);
    auto [seconds, cycles] = SharedVerifier::endClock();
    std::cout << seconds << " Seconds, " << cycles << " Clock Cycles";
    std::cout << ", " << !result << " Result Code" << std::endl;
    return !result;
}