
#pragma once

#include "Proof.hpp"

namespace lazyslate {

    struct Node {
        size_t id;
        std::string name;
        std::string justification;
        std::string expression;
        size_t x, y;
    };

    struct Link {
        size_t fromNodeId;
        size_t toNodeId;
    };

    struct FileData {
        std::vector<Node> nodes;
        std::vector<Link> links;
    };

    FileData parse(const std::string& fileContents);
    Proof constructProof(const FileData&);

}