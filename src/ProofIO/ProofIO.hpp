

#pragma once

#include "../Proof/Proof.hpp"
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

    ProofData loadProofData(std::string filename);
    Proof loadProofFromFile(std::string filename);
    Proof loadFromJSONContents(const std::string& fileContents);
}