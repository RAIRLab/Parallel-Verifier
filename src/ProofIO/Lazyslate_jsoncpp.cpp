
//This file implements the construction of lazyslate proof parsing via libjsoncpp json parsing

#include <json/json.h>

#include "Lazyslate.hpp"

#include "Proof.hpp"

using namespace lazyslate;

void throwFailure(const std::string& message){
    throw std::runtime_error("ProofIO Error: Error reading lazyslate " \
    "json file: " + message);
}

FileData lazyslate::parse(const std::string& fileContents) {
    lazyslate::FileData returnData;
    Json::Value root;
    Json::Reader reader;
    bool success = reader.parse(fileContents, root, false);
    if(!success || !root || !root.isObject()) {
        std::cerr<<reader.getFormattedErrorMessages()<<std::endl;
        throwFailure("failed to parse json file");
    }

    //Process Nodes
    const Json::Value nodes = root["nodes"];
    if(!nodes || !nodes.isArray()) {
        throwFailure("node array corrupted");
    }

    for(const Json::Value& node : nodes) {
        Node proofNode;
        Json::Value id = node["id"];
        Json::Value name = node["name"];
        Json::Value expression = node["expression"];
        Json::Value justification = node["justification"];
        Json::Value position = node["position"];
        if(!position || !position.isObject()) {
            throwFailure("node position corrupted");
        }
        Json::Value xPosition = position["x"];
        Json::Value yPosition = position["y"];
        if(!id || !id.isIntegral()
        || !name || !(name.isString() || name.isIntegral())
        || !expression || !expression.isString()
        || !justification || !justification.isString()
        || !xPosition  || !xPosition.isIntegral()
        ){
            throwFailure("node corrupted");
        }
        proofNode.id = node["id"].asUInt64();
        if(name.isIntegral()){
            proofNode.name = std::to_string(name.asUInt64());
        }else{
            proofNode.name = name.asString();
        }
        proofNode.expression = expression.asString();
        proofNode.justification = justification.asString();
        proofNode.x = xPosition.asUInt64();
        proofNode.y = yPosition.asUInt64();
        returnData.nodes.push_back(proofNode);
    }

    //Process links 
    const Json::Value links = root["links"];
    if(!links || !links.isArray()) {
        throwFailure("link array corrupted");
    }

    for(const Json::Value& jsonLink : links) {
        Link proofLink;
        if(!jsonLink || !jsonLink.isArray() || jsonLink.size() != 2){
            throwFailure("link corrupted");
        }
        Json::Value fromNodeId = jsonLink[0];
        Json::Value toNodeId = jsonLink[1];
        if(!fromNodeId || !fromNodeId.isIntegral()
        || !toNodeId || !toNodeId.isIntegral()
        ){
            throwFailure("link corrupted");
        }
        proofLink.fromNodeId = fromNodeId.asUInt64();
        proofLink.toNodeId = toNodeId.asUInt64();
        returnData.links.push_back(proofLink);
    }
    
    return returnData;
}


