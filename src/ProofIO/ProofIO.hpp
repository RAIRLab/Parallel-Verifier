

#pragma once

#include "../Proof.hpp"
#include "HyperSlate.hpp"
#include "Lazyslate.hpp"

namespace ProofIO {

    struct ProofData {
        enum Tag {Hyperslate, Lazyslate} tag;
        union {
            hyperslate::FileData hyperslateData;
            lazyslate::FileData lazyslateData;
        };
    };

    ProofData loadData(std::string filename);
    Proof loadProof(std::string filename);
}