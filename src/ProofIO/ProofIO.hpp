

#pragma once

#include <variant>

#include "../Proof/Proof.hpp"
#include "HyperSlate.hpp"
#include "Lazyslate.hpp"

namespace ProofIO {

    enum class ProofDataType{
        HyperSlate = 0,
        LazySlate = 1
    };

    using ProofData = std::variant<hyperslate::FileData, lazyslate::FileData>;

    //Read from file and construct based on file extension (.slt or .json)
    ProofData loadProofData(std::string filename);
    Proof loadProofFromFile(std::string filename);

    //Construct based on file contents (first char is a "{" or a "(")
    Proof loadFromContents(const std::string& fileContents,
                           std::optional<ProofDataType> type = std::nullopt);

    Proof loadFromJSONContents(const std::string& fileContents);

    void toLazyslateFile(std::string filename, const Proof& proof);
}