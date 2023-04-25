
#James Oswald 

from pathlib import Path

#Hyperparamters ======================================================


#Tree proof Topology
treeDepths: "list(int)" = [2, 4, 6, 8, 10, 12]  #The tree will have 2^treeDepth nodes


#writes a hyperlate proof .slt file given a list of descriptions and connections
def writeSltFile(fileName: str, descriptions : "list(str)", connections: "list(str)"):
    file = open(fileName, "w")
    file.write("(:DESCRIPTIONS (")
    for description in descriptions:
        file.write(description + " ")
    file.write(") :STRUCTURES (")
    for structure in connections:
        file.write(structure + " ")
    file.write(") :INTERFACE (:X 0 :Y 0 :WIDTH 0 :HEIGHT 0 :PROOF-SYSTEM 0) :CONNECTOR-TYPE \"\" :BACKGROUND-COLOR \"\")")
    file.close()

#generate the tree benchmark
def genTreeFormulas(formulas : "list(str)", descriptions : "list(str)", structures : "list(str)", treeDepth:int) -> int:
    myIndex = len(formulas)
    formulas.append("")
    descriptions.append("")
    if treeDepth == 1:
        formulas[myIndex] = "A" + str(myIndex)
        descriptions[myIndex] = f"(:X 0 :Y 0 :ID {myIndex} :NAME \"{myIndex}\" :FORMULA \"{formulas[myIndex]}\" :JUSTIFICATION LOGIC::ASSUME)"
        return myIndex
    else:    
        leftChildIndex = genTreeFormulas(formulas, descriptions, structures, treeDepth-1)
        rightChildIndex = genTreeFormulas(formulas, descriptions, structures, treeDepth-1)
        formulas[myIndex] = f"(and {formulas[leftChildIndex]} {formulas[rightChildIndex]})"
        descriptions[myIndex] = f"(:X 0 :Y 0 :ID {myIndex} :NAME \"{myIndex}\" :FORMULA \"{formulas[myIndex]}\" :JUSTIFICATION LOGIC::AND-INTRO)"
        structures.append(f"(:CONCLUSION {myIndex} :PREMISES ({leftChildIndex} {rightChildIndex}))")
        return myIndex

def genTreeTopology(treeLevels: int):
    formulas : "list(str)" = []
    descriptions : "list(str)" = []
    connections : "list(str)" = []
    genTreeFormulas(formulas, descriptions, connections, treeLevels)
    writeSltFile(f"benchmarks/LFTree{treeLevels}.slt", descriptions, connections)


Path("benchmarks").mkdir(parents=True, exist_ok=True)

for td in treeDepths:
    genTreeTopology(td)