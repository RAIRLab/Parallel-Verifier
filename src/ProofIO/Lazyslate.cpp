

#include <json/json.h>

#include "Lazyslate.hpp"

#include "../Proof.hpp"


using namespace lazyslate;


FileData lazyslate::parse(const std::string& fileContents) {
    lazyslate::FileData returnData;
    Json::Value root;
    Json::Reader reader;
    reader.parse(fileContents, root, false);
    const Json::Value nodes = root["nodes"];
    const Json::Value links = root["links"];
    if(!nodes || !nodes.isArray() || !links || !links.isArray()) {
        throw std::runtime_error("ProofIO Error: Error reading lazyslate " \
        "json file, nodes or proofs invalid");
    }
    for(const Json::Value& node : nodes) {
        Node proofNode;
        Json::Value id = node["id"];
        Json::Value name = node["name"];
        Json::Value expression = node["expression"];
        Json::Value justification = node["justification"];
        Json::Value position = node["position"];
        if(!position || !position.isObject()) {
            throw std::runtime_error("ProofIO Error: Error reading lazyslate " \
            "json file, a node position is invalid");
        }
        Json::Value xPosition = position["x"];
        Json::Value yPosition = position["y"];
        if(!id || !id.isUInt64()
        || !name || !(name.isString() || name.isIntegral());
        || !expression || !expression.isString()
        || !justification || !justification.isString()
        || !xPosition  
        )
        proofNode.id = node["id"].asUInt64();

    }
}

Proof lazyslate::constructProof(const FileData&) {

}
