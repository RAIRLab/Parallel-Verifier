

#include<unordered_map>
#include<string>

#include "../SExpression.hpp"
#include "HyperSlate.hpp"

//Maps hyperslate justification strings to hyperslate justification enums
const std::unordered_map<std::string, hyperslate::Justification> \
justificationMap = {
    {"LOGIC::ASSUME", hyperslate::Justification::Assume}, 
    {"LOGIC::AND-INTRO", hyperslate::Justification::AndIntro}, 
    {"LOGIC::AND-ELIM", hyperslate::Justification::AndElim}, 
    {"LOGIC::OR-INTRO", hyperslate::Justification::OrIntro}, 
    {"LOGIC::OR-ELIM", hyperslate::Justification::OrElim},
    {"LOGIC::NOT-INTRO", hyperslate::Justification::NotIntro}, 
    {"LOGIC::NOT-ELIM", hyperslate::Justification::NotElim}, 
    {"LOGIC::IF-INTRO", hyperslate::Justification::IfIntro}, 
    {"LOGIC::IF-ELIM", hyperslate::Justification::IfElim}, 
    {"LOGIC::IFF-INTRO", hyperslate::Justification::IffIntro}, 
    {"LOGIC::IFF-ELIM", hyperslate::Justification::IffElim},
    {"LOGIC::=-INTRO", hyperslate::Justification::EqIntro}, 
    {"LOGIC::=-ELIM", hyperslate::Justification::EqElim}, 
    {"LOGIC::FORALL-INTRO", hyperslate::Justification::ForallIntro}, 
    {"LOGIC::FORALL-ELIM", hyperslate::Justification::ForallElim},
    {"LOGIC::EXISTS-INTRO", hyperslate::Justification::ExistsIntro}, 
    {"LOGIC::EXISTS-ELIM", hyperslate::Justification::ExistsElim}, 
    {"LOGIC::PC-PROVABILITY", hyperslate::Justification::PCOracle},
    {"LOGIC::FOL-PROVABILITY", hyperslate::Justification::FOLOracle}
};

/*
This function takes a Hyperslate Justification and converts it to "Proof
Justification" IE, a justification that our prover supports. It depends on
hyperslate::Justifications having the same underlying enum values as 
Proof::Justifications for all supported values
*/
Proof::Justification getProofJustification(hyperslate::Justification just){
    unsigned int enumValue = static_cast<unsigned int>(just);
    if (enumValue >= static_cast<unsigned int>(Proof::Justification::COUNT)) {
        throw std::runtime_error("Hyperslate Conversion Error: This " \
        " justification is not yet implemented: Justification #" + enumValue);
    }
    return static_cast<Proof::Justification>(enumValue);
}

/*
Note:
Reimplement this with struct constructors at some point, this C style
implementation is jank
-Old James
No I don't think I will
-New James

Takes a path and stores all its data in a C struct with structure and typing 
*/
hyperslate::FileData parseFile(const std::string& fileContents){
    sExpression fileExpression(fileContents);

    //Pull the descriptions
    std::vector<hyperslate::Description> returnDescriptions;
    sExpression descriptions = fileExpression["DESCRIPTIONS"];
    for(sExpression& description : descriptions.members){
        hyperslate::Description returnDescription;
        returnDescription.x = std::stoi(description["X"].value);
        returnDescription.y = std::stoi(description["Y"].value);
        returnDescription.id = std::stoi(description["ID"].value);
        returnDescription.name = description["NAME"].value;
        returnDescription.formula = description["FORMULA"].value;
        std::string justString = description["JUSTIFICATION"].value;
        returnDescription.justification = justificationMap.at(justString);
        returnDescriptions.push_back(returnDescription);
    }

    //Pull the structures
    std::vector<hyperslate::Structure> returnStructures;
    sExpression structures = fileExpression["STRUCTURES"];
    for(sExpression& structure : structures.members){
        hyperslate::Structure returnStructure;
        std::vector<vertId> returnPremises;
        for(const sExpression& premise : structure["PREMISES"].members)
            returnPremises.push_back(std::stoi(premise.value));
        returnStructure.premises = returnPremises;
        returnStructure.conclusion = std::stoi(structure["CONCLUSION"].value);
        returnStructures.push_back(returnStructure);
    }

    //Read the interface
    hyperslate::Interface returnInterface;
    sExpression interface = fileExpression["INTERFACE"];
    returnInterface.x = std::stoi(interface["X"].value);
    returnInterface.y = std::stoi(interface["Y"].value);
    returnInterface.width = std::stoi(interface["WIDTH"].value);
    returnInterface.height = std::stoi(interface["HEIGHT"].value);
    returnInterface.proofSystem = interface["PROOF-SYSTEM"].value;

    hyperslate::FileData returnData;
    returnData.descriptions = returnDescriptions;
    returnData.structures = returnStructures;
    returnData.interface = returnInterface;
    returnData.connectorType = fileExpression["CONNECTOR-TYPE"].value;
    returnData.backgroundColor = fileExpression["BACKGROUND-COLOR"].value;

    return returnData;
}

Proof constructProof(const hyperslate::FileData& fileData){
    Proof proof;
    proof.nodeLookup = std::unordered_map<vertId, Proof::Node>();
    for(hyperslate::Description descriptionNode : fileData.descriptions){
        if(descriptionNode.formula != ""){  //Ignore selmers comment nodes
            Proof::Node proofNode(
                descriptionNode.id,
                sExpression(descriptionNode.formula),
                getProofJustification(descriptionNode.justification)
            );
            if(proofNode.justification == Proof::Justification::Assume)
                proof.assumptions.insert(proofNode.id);
            proof.nodeLookup[proofNode.id] = proofNode;
        }
    }
    for(hyperslate::Structure connection : fileData.structures){
        for(vertId premise : connection.premises){
            proof.nodeLookup[premise].children.insert(connection.conclusion);
            proof.nodeLookup[connection.conclusion].parents.insert(premise);
        }
    }
}

