

#include<unordered_map>
#include<string>
#include<fstream>

#include "SExpression.hpp"
#include "hyperslateUtils.hpp"

//Maps hyperslate justification strings to our justification enums
const std::unordered_map<std::string, HyperslateJustification> justificationMap = {
    {"LOGIC::ASSUME", Assume}, 
    {"LOGIC::AND-INTRO", AndIntro}, 
    {"LOGIC::AND-ELIM", AndElim}, 
    {"LOGIC::OR-INTRO", OrIntro}, 
    {"LOGIC::OR-ELIM", OrElim},
    {"LOGIC::NOT-INTRO", NotIntro}, 
    {"LOGIC::NOT-ELIM", NotElim}, 
    {"LOGIC::IF-INTRO", IfIntro}, 
    {"LOGIC::IF-ELIM", IfElim}, 
    {"LOGIC::IFF-INTRO", IffIntro}, 
    {"LOGIC::IFF-ELIM", IffElim},
    {"LOGIC::=-INTRO", EqIntro}, 
    {"LOGIC::=-ELIM", EqElim}, 
    {"LOGIC::FORALL-INTRO", ForallIntro}, 
    {"LOGIC::FORALL-ELIM", ForallElim},
    {"LOGIC::EXISTS-INTRO", ExistsIntro}, 
    {"LOGIC::EXISTS-ELIM", ExistsElim}, 
    {"LOGIC::PC-PROVABILITY", PCOracle}, 
    {"LOGIC::FOL-PROVABILITY", FOLOracle}
};

//Reads a file at path into a string and returns it
std::string readFile(std::string path){
    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

//Reimplement this with struct constructors at some point, this C style implementation is jank
//Takes a path and stores all its data in a C struct with better structure and typing 
HyperslateFileData parseHyperslateFile(std::string path){
    
    //Read the file and convert it to an S expression
    std::string fileContents = readFile(path);
    sExpression fileExpression(fileContents);

    //Pull the descriptions
    std::vector<HyperslateDescription> returnDescriptions;
    sExpression descriptions = fileExpression["DESCRIPTIONS"];
    for(sExpression& description : descriptions.members){
        HyperslateDescription returnDescription;
        returnDescription.x = std::stoi(description["X"].value);
        returnDescription.y = std::stoi(description["Y"].value);
        returnDescription.id = std::stoi(description["ID"].value);
        returnDescription.name = description["NAME"].value;
        returnDescription.formula = description["FORMULA"].value;
        returnDescription.justification = justificationMap.at(description["JUSTIFICATION"].value);
        returnDescriptions.push_back(returnDescription);
    }

    //Pull the structures
    std::vector<HyperslateStructure> returnStructures;
    sExpression structures = fileExpression["STRUCTURES"];
    for(sExpression& structure : structures.members){
        HyperslateStructure returnStructure;
        std::vector<id_t> returnPremises;
        for(const sExpression& premise : structure["PREMISES"].members)
            returnPremises.push_back(std::stoi(premise.value));
        returnStructure.premises = returnPremises;
        returnStructure.conclusion = std::stoi(structure["CONCLUSION"].value);
        returnStructures.push_back(returnStructure);
    }

    //Read the interface
    HyperslateInterface returnInterface;
    sExpression interface = fileExpression["INTERFACE"];
    returnInterface.x = std::stoi(interface["X"].value);
    returnInterface.y = std::stoi(interface["Y"].value);
    returnInterface.width = std::stoi(interface["WIDTH"].value);
    returnInterface.height = std::stoi(interface["HEIGHT"].value);
    returnInterface.proofSystem = interface["PROOF-SYSTEM"].value;

    HyperslateFileData returnData;
    returnData.descriptions = returnDescriptions;
    returnData.structures = returnStructures;
    returnData.interface = returnInterface;
    returnData.connectorType = fileExpression["CONNECTOR-TYPE"].value;
    returnData.backgroundColor = fileExpression["BACKGROUND-COLOR"].value;

    return returnData;
}
