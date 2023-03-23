
#pragma once

#include "../Proof/Proof.hpp"

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

    const std::unordered_map<std::string, Proof::Justification> \
    justificationMap = {
        {"assume", Proof::Justification::Assume}, 
        {"andI", Proof::Justification::AndIntro}, 
        {"andE", Proof::Justification::AndElim}, 
        {"orI", Proof::Justification::OrIntro}, 
        {"orE", Proof::Justification::OrElim},
        {"notI", Proof::Justification::NotIntro}, 
        {"notE", Proof::Justification::NotElim}, 
        {"ifI", Proof::Justification::IfIntro}, 
        {"ifE", Proof::Justification::IfElim}, 
        {"iffI", Proof::Justification::IffIntro}, 
        {"iffE", Proof::Justification::IffElim},
        {"forallI", Proof::Justification::ForallIntro}, 
        {"forallE", Proof::Justification::ForallElim},
        {"existsI", Proof::Justification::ExistsIntro}, 
        {"existsE", Proof::Justification::ExistsElim}, 
    };

    const std::unordered_map<Proof::Justification, std::string> \
    reverseJustificationMap = {
        {Proof::Justification::Assume, "assume"}, 
        {Proof::Justification::AndIntro, "andI"}, 
        {Proof::Justification::AndElim, "andE"}, 
        {Proof::Justification::OrIntro, "orI"}, 
        {Proof::Justification::OrElim, "orE"},
        {Proof::Justification::NotIntro, "notI"}, 
        {Proof::Justification::NotElim, "notE"}, 
        {Proof::Justification::IfIntro, "ifI"}, 
        {Proof::Justification::IfElim, "ifE"}, 
        {Proof::Justification::IffIntro, "iffI"}, 
        {Proof::Justification::IffElim, "iffE"},
        {Proof::Justification::ForallIntro, "forallI"}, 
        {Proof::Justification::ForallElim, "forallE"},
        {Proof::Justification::ExistsIntro, "existsI"}, 
        {Proof::Justification::ExistsElim, "existsE"} 
    };
}