
#ifndef HYPERSLATEUTILS_H
#define HYPERSLATEUTILS_H


#include<string>
#include<vector>

//Possible justifications (infrence rules) on a node
enum HyperslateJustification{
    Assume, AndIntro, AndElim, OrIntro, OrElim,
    NotIntro, NotElim, IfIntro, IfElim, IffIntro, IffElim,
    EqIntro, EqElim, ForallIntro, ForallElim,
    ExistsIntro, ExistsElim, PCOracle, FOLOracle
};

typedef unsigned int id_t;

struct HyperslateDescription{
    int x, y;                       //positions of the node in the hyperslate UI
    id_t id;                        //Internal ID in hyperstate
    std::string name;               //Hyperslate names
    std::string formula;            //The textual representation of the formula
    HyperslateJustification justification;      //The logical justification
};

struct HyperslateStructure{
    id_t conclusion;
    std::vector<id_t> premises;
};

struct HyperslateInterface{
    int x, y;            //The cordinates of the top left workspace corner 
    int width, height;   //The offset of the bot right workspace corner relative to x, y
    std::string proofSystem; 
};

struct HyperslateFileData{
    std::vector<HyperslateStructure> structures;    //The nodes (including justifications)
    std::vector<HyperslateDescription> descriptions; //The connections between nodes
    
    //For some unknown reason, in hyperslate files, :BACKGROUND-COLOR and :CONNECTOR-TYPE
    //are on the same level as :DESCRIPTIONS and :STRUCTURES, and not included in the 
    //:INTERFACE. We model this in the struct because sure why not. 

    HyperslateInterface interface; 
    std::string connectorType;      
    std::string backgroundColor;
};

HyperslateFileData parseHyperslateFile(std::string fileContents);

#endif