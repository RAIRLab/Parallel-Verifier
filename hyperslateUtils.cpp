


#include<string>
#include<fstream>

#include "SExpression.hpp"
#include "hyperslateUtils.hpp"

std::string readFile(const std::string& path){
    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

HyperslateFileData parseHyperslateFile(const std::string&& path){
    
    std::string fileContents = readFile(path);
    sExpression fileExpression(fileContents);


    std::vector<HyperslateDescription> returnDescriptions;
    sExpression descriptions = fileExpression["DESCRIPTIONS"];
    for(sExpression& description : descriptions.members){
        HyperslateDescription returnDescription;
        returnDescription.x = std::stoi(description["X"].value);
        returnDescription.y = std::stoi(description["Y"].value);
        returnDescription.id = std::stoi(description["ID"].value);
        returnDescription.name = description["NAME"].value;
        returnDescription.formula = description["FORMULA"].value;
        returnDescription.justification = description["JUSTIFICATION"].value;
        returnDescriptions.push_back(returnDescription);
    }

    
    std::vector<HyperslateDescription> returnStructures;
    sExpression structures = fileExpression["STRUCTURES"];
    for(sExpression& structure : structures.members){
        HyperslateStructure returnStructure;
        returnStructure.conclusion = 
        returnStructures.push_back()
    }

    HyperslateInterface returnInterface;
    sExpression interface = fileExpression["INTERFACE"];
    returnInterface.x = std::stoi(interface["X"].value);
    returnInterface.y = std::stoi(interface["Y"].value);
    returnInterface.width = std::stoi(interface["WIDTH"].value);
    returnInterface.height = std::stoi(interface["HEIGHT"].value);
    returnInterface.proofSystem = interface["PROOF-SYSTEM"].value;

    HyperslateFileData returnData;

    returnData.interface = returnInterface;
    returnData.connectorType = fileExpression["CONNECTOR-TYPE"].value;
    returnData.backgroundColor = fileExpression["BACKGROUND-COLOR"].value;

    return returnData;
}
