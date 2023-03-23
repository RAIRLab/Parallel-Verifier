

#include "Lazyslate.hpp"

using lazyslate::justificationMap;

Proof lazyslate::constructProof(const lazyslate::FileData& fileData){
    Proof proof;
    proof.nodeLookup = std::unordered_map<VertId, Proof::Node>();
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