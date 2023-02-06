

#include "Lazyslate.hpp"

#include "../Proof.hpp"

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

Proof lazyslate::constructProof(const lazyslate::FileData& fileData){
    Proof proof;
    proof.nodeLookup = std::unordered_map<vertId, Proof::Node>();
    for(lazyslate::Node node : fileData.nodes){
        if(justificationMap.find(node.justification) == justificationMap.end())
            throw std::runtime_error("ProofIO Error: proof construction "\
            "error, invalid proof justification");

        Proof::Node proofNode(node.id, sExpression(node.expression),
                              justificationMap.at(node.justification));
        if(proofNode.justification == Proof::Justification::Assume)
            proof.assumptions.insert(proofNode.id);
        proof.nodeLookup[proofNode.id] = proofNode;
    }
    for(lazyslate::Link link : fileData.links){
        size_t fromId = link.fromNodeId;
        size_t toId = link.toNodeId;
        proof.nodeLookup[fromId].children.insert(toId);
        proof.nodeLookup[toId].parents.insert(fromId);
    }
    return proof;
}