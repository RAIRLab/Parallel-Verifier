

#pragma once

#include <variant>

#include "../Proof/Proof.hpp"
#include "HyperSlate.hpp"
#include "Lazyslate.hpp"

namespace ProofIO {

    using ProofData = std::variant<hyperslate::FileData, lazyslate::FileData>;

    ProofData loadProofData(std::string filename);
    Proof loadProofFromFile(std::string filename);
    Proof loadFromJSONContents(const std::string& fileContents);

    void toLazyslateFile(std::string filename, const Proof& proof);
}