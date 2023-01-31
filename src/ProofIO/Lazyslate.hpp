

namespace lazyslate {

    struct FileData : public ProofData{
        std::vector<Structure> structures;    //The nodes (including justifications)
        std::vector<Description> descriptions; //The connections between nodes
        
        //For some unknown reason, in hyperslate files, :BACKGROUND-COLOR and :CONNECTOR-TYPE
        //are on the same level as :DESCRIPTIONS and :STRUCTURES, and not included in the 
        //:INTERFACE. We model this in the struct because sure why not. 

        Interface interface; 
        std::string connectorType;      
        std::string backgroundColor;
    };

}