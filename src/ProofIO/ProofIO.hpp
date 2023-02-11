

#pragma once

#include "Proof.hpp"
#include "HyperSlate.hpp"
#include "Lazyslate.hpp"

namespace ProofIO {

    struct ProofData {
        ProofData();
        ~ProofData();
        ProofData(const ProofData& copy);
        enum Tag {Hyperslate, Lazyslate} tag;

        //This would be a tagged union if not for
        //https://stackoverflow.com/a/65190895/6342516
        hyperslate::FileData hyperslateData;
        lazyslate::FileData lazyslateData;
    };

    ProofData loadData(std::string filename);
    Proof loadProof(std::string filename);
}