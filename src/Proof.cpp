
#include"Proof.hpp"

//This lets us construct the proof from the raw .slt file contents
Proof::Proof(std::string slateFileContents)
:Proof(parseHyperslateFile(slateFileContents))
{}

Proof::Proof(HyperslateFileData fileData){
    nodeLookup = std::unordered_map<vertId, ProofNode>();
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
        for(vertId premise : connection.premises){
            nodeLookup[premise].children.insert(connection.conclusion);
            nodeLookup[connection.conclusion].parents.insert(premise);
        }
    }
}