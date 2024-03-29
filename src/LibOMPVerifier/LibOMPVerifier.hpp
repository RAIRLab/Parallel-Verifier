
#pragma once

#include "../Proof/Proof.hpp"

namespace OMPVerifier {
    bool OMPVerifyOriginal(const Proof& p);
    bool OMPVerifyLB(const Proof& p);  
    bool OMPVerifyBF(const Proof& p); 
}