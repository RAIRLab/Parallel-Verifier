
import random

from SAFormula.Atom import Atom
from SAFormula.And import And
from SAFormula.Or import Or
from SAFormula.Not import Not
from SAFormula.Implication import Implication
from SAFormula.BiConditional import BiConditional

def constructNode( 
    id : int,
    name : str,
    formula: str,
    justif : str,
    posx : int,
    posy : int 
):
    return {
        "id" : id,
        "name" : name,
        "formula" : formula,
        "justification" : justif,
        "position:" : {"x" : posx, "y" : posy} 
    }


introRules = [
    "andI",
    "orI",
    "ifI",
    "iffI",
    "notI",
]

elimRules = [
    "andE",
    "orE",
    "ifE",
    "iffE",
    "notE",
]

curNode = 0
nodeMap = {} #map of all nodes by id
assumptionMap = {} #maps node id to list of assuptions on node
SAFormulaMap = {} #maps node IDs to its SA Formula

#Takes the current layer and applies intros nodes for the next layer
# until out of nodes 
#Returns the bottom most node that will have stuff added 
#to it in the next layer
#global variables
def introRandom(nodeId, curLayer, layerNum):
    counter = 0
    while len(curLayer) > 0:
        rule = random.choice(introRules)
        match rule:
            case "andI":
                n1 = nodeId
                curLayer.remove(nodeId)
                n2 = random.choice(curLayer)
                
                curLayer.remove(n2)
                SAFormulaMap[curNode] = And.from_args(SAFormulaMap[n1], 
                                                      SAFormulaMap[n2])
                assumptionMap[curNode] = assumptionMap[n1] + assumptionMap[n2]
                nodeMap[curNode] = constructNode(
                    curNode, str(curNode),
                    str(SAFormulaMap[curNode]),
                    "andI", counter * 200, layerNum * 200) 
            case "orI":
                
            case "notI":

            case "ifI":

            case "iffI":
        curLayer.remove(nodeId)
        counter += 1
    


    

def genNullModels(initialNodeCount, layers):
    curLayer = []
    for i in range(initialNodeCount):
        nodeMap[i] = constructNode(i, str(i), "P" + str(i), "assume", i * 200, 0)
        assumptionMap[i] = [i]
        SAFormulaMap[i] = Atom.from_string("P" + str(i))
        curLayer.append(i)
        curNode += 1

    nextLayer = []
    for layer in range(1, layers):
        for nodeId in curLayer:
            if(nodeMap[nodeId]["justification"] == "assume"):
                introRandom(nodeId, curLayer, layer)



        nextLayer = curLayer
        