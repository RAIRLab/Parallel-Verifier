
#include"Proof.hpp"

Proof::Proof(HyperslateFileData fileData){
    nodeLookup = std::vector<ProofNode>(fileData.descriptions.size() + 1);
    for(HyperslateDescription descriptionNode : fileData.descriptions){
        ProofNode proofNode;
        proofNode.id = descriptionNode.id;
        proofNode.justification = descriptionNode.justification;
        if(proofNode.justification == HyperslateJustification::Assume)
            assumptions.insert(proofNode.id);
        proofNode.formula = sExpression(descriptionNode.formula);
        nodeLookup[proofNode.id] = proofNode;
    }
    for(HyperslateStructure connection : fileData.structures){
        for(id_t premise : connection.premises){
            nodeLookup[premise].children.insert(connection.conclusion);
        }
    }
}