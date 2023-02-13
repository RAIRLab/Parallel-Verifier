
#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Proof.hpp"

namespace hyperslate { 

    //Possible justifications (inference rules) on a node
    enum class Justification{
        Assume, AndIntro, AndElim, OrIntro, OrElim,
        NotIntro, NotElim, IfIntro, IfElim, IffIntro, IffElim,
        EqIntro, EqElim, ForallIntro, ForallElim,
        ExistsIntro, ExistsElim, PCOracle, FOLOracle,
    };

    struct Description{
        int x, y;                       //positions of the node in the hyperslate UI
        VertId id;                        //Internal ID in hyperslate
        std::string name;               //Hyperslate names
        std::string formula;            //The textual representation of the formula
        Justification justification;      //The logical justification
    };

    struct Structure{
        VertId conclusion;
        std::vector<VertId> premises;
    };

    struct Interface{
        int x, y;            //The coordinates of the top left workspace corner 
        int width, height;   //The offset of the bot right workspace corner relative to x, y
        std::string proofSystem; 
    };

    struct FileData{
        std::vector<Structure> structures;    //The nodes (including justifications)
        std::vector<Description> descriptions; //The connections between nodes
        
        //For some unknown reason, in hyperslate files, :BACKGROUND-COLOR and :CONNECTOR-TYPE
        //are on the same level as :DESCRIPTIONS and :STRUCTURES, and not included in the 
        //:INTERFACE. We model this in the struct because sure why not. 

        Interface interface; 
        std::string connectorType;      
        std::string backgroundColor;
    };

    FileData parse(const std::string& fileContents);
    Proof constructProof(const hyperslate::FileData& fileData);
};
