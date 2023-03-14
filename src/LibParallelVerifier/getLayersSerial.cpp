
#include"LibParallelVerifier.hpp"



//Vector version of layer map
using LayerVector = std::vector<std::unordered_set<VertId>>;


//Recursive helper for getLayerAndDepthMapsSerial
//Returns the depth of a node id, in a proof p, using and modifying 
//a global depth map for the proof.
size_t getDepth(const Proof& p, VertId id, DepthMap& depthMap){
    //Base case 1, we already know the depth of the node
    DepthMap::const_iterator depthMapIter = depthMap.find(id);
    if(depthMapIter != depthMap.end()){
        return depthMapIter->second;
    }
    //Base case 2, we're an assumption
    std::unordered_set<VertId> parents = p.nodeLookup.at(id).parents;
    if(parents.size() == 0){
        depthMap[id] = 0;

        return 0;
    }

    std::unordered_set<int>::const_iterator itr = parents.begin();
    size_t maxDepth = getDepth(p, *itr, depthMap);
    itr++;
    for(;itr != parents.end(); itr++){
        size_t curDepth = getDepth(p, *itr, depthMap);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }
    depthMap[id] = maxDepth + 1;
    return maxDepth + 1;
}

//O(n) serial construction of the depth and layer maps for a proof.
//n is the number of nodes in the proof 
std::pair<LayerMap, DepthMap> ParallelVerifier::getLayerMapSerial(
                                                            const Proof& p){
    //Depthmap construction is O(n) via dynamic programming
    DepthMap depthMap;
    size_t maxDepth = 0;
    for(const auto& [id, node] : p.nodeLookup){
        size_t curDepth = getDepth(p, id, depthMap);
        if(curDepth > maxDepth){
            maxDepth = curDepth;
        }
    }

    //LayerMap construction O(n)
    //std::list fill constructor
    LayerVector layerVector(maxDepth + 1, std::unordered_set<VertId>());
    for(const auto& [id, depth] : depthMap){
        layerVector[depth].insert(id);
    }
    LayerMap layerMap(layerVector.begin(), layerVector.end());

    return std::make_pair(layerMap, depthMap);
}