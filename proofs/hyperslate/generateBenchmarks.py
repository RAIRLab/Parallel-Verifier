
#James Oswald

from pathlib import Path

#Hyperparamters ======================================================

#Line Proof Topology
lineLengths: "list(int)" = [100, 150, 200, 250, 300, 350, 400] #Number of nodes in the the linear benchmark

#Branch Proof Topology
numBranchs: int = 12
branchLengths: "list(int)" = [1, 25, 50, 75, 100, 125, 150]

#Tree proof Topology
treeDepths: "list(int)" = [8, 10, 12, 14, 16, 18, 20]  #The tree will have 2^treeDepth nodes


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

#generates a line of formulas using ors
def genLineFormulas(symbol: str, formulas : "list(str)", depth: int):
    if depth == 1:
        formulas.append(symbol)
    else:
        genLineFormulas(symbol, formulas, depth-1)
        formulas.append(f"(or {symbol} {formulas[-1]})")

def genLineTopology(lineLength):
    lineFormulas : "list(str)" = []
    genLineFormulas("A", lineFormulas, lineLength)
    descriptions : "list(str)" = []
    connections : "list(str)" = []
    for i, formula in enumerate(lineFormulas):
        if i == 0:
            descriptions.append(f"(:X 0 :Y 0 :ID {i} :NAME \"{i}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::ASSUME)")
        if i != 0:
            connections.append(f"(:CONCLUSION {i} :PREMISES ({i-1}))")
            descriptions.append(f"(:X 0 :Y 0 :ID {i} :NAME \"{i}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::OR-INTRO)")
    writeSltFile(f"benchmarks/Strait{lineLength}.slt", descriptions, connections)


#Generate Branch Topology Proofs
def genBranchTopology(branchLength, numBracnhs):
    symbols = ["A" + str(i) for i in range(numBracnhs)]
    descriptions : "list(str)" = []
    connections : "list(str)" = []
    allFormulas : "list(str)" = [] #The formula for each vert with respect to its vert id as its index
    baseVertIdIndex = 0
    tieInIndexs: "list(int)" = [] #The indexes at the bottom of each brach which will be tied togeather to form the final conclusion
    for s in symbols:
        lineFormulas : "list(str)" = []
        genLineFormulas(s, lineFormulas, branchLength)
        allFormulas += lineFormulas
        for i, formula in enumerate(lineFormulas):
            vertId = baseVertIdIndex+i
            if i == 0:
                descriptions.append(f"(:X 0 :Y 0 :ID {vertId} :NAME \"{vertId}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::ASSUME)")
            if i != 0:
                connections.append(f"(:CONCLUSION {vertId} :PREMISES ({vertId-1}))")
                descriptions.append(f"(:X 0 :Y 0 :ID {vertId} :NAME \"{vertId}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::OR-INTRO)")
        baseVertIdIndex += len(lineFormulas)
        tieInIndexs.append(baseVertIdIndex - 1)

    #Tying the branches to a single conclusion
    lastFormula : str = ""
    lastFormulaId : int = 0
    vertId = baseVertIdIndex
    for i, ti in enumerate(tieInIndexs):
        if i == 0:
            lastFormulaId = ti
            lastFormula = allFormulas[ti]
        elif i != 0:
            lastFormula = f"(and {lastFormula} {allFormulas[ti]})"
            descriptions.append(f"(:X 0 :Y 0 :ID {vertId} :NAME \"{vertId}\" :FORMULA \"{lastFormula}\" :JUSTIFICATION LOGIC::AND-INTRO)")
            connections.append(f"(:CONCLUSION {vertId} :PREMISES ({ti} {lastFormulaId}))")
            lastFormulaId = vertId
            vertId+=1
    writeSltFile(f"benchmarks/Branch{numBracnhs}-{branchLength}.slt", descriptions, connections)

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
        formulas.append("")
        descriptions.append("")
        leftChildIndex = genTreeFormulas(formulas, descriptions, structures, treeDepth-1)
        rightChildIndex = genTreeFormulas(formulas, descriptions, structures, treeDepth-1)

        # Conjunction Introduction
        formulas[myIndex] = f"(and {formulas[leftChildIndex]} {formulas[rightChildIndex]})"
        descriptions[myIndex] = f"(:X 0 :Y 0 :ID {myIndex} :NAME \"{myIndex}\" :FORMULA \"{formulas[myIndex]}\" :JUSTIFICATION LOGIC::AND-INTRO)"
        structures.append(f"(:CONCLUSION {myIndex} :PREMISES ({leftChildIndex} {rightChildIndex}))")

        # Conjunction Elimination
        myIndex += 1
        formulas[myIndex] = f"{formulas[leftChildIndex]}"
        descriptions[myIndex] = f"(:X 0 :Y 0 :ID {myIndex} :NAME \"{myIndex}\" :FORMULA \"{formulas[myIndex]}\" :JUSTIFICATION LOGIC::AND-ELIM)"
        structures.append(f"(:CONCLUSION {myIndex} :PREMISES ({myIndex - 1}))")
        return myIndex

def genTreeTopology(treeLevels: int):
    formulas : "list(str)" = []
    descriptions : "list(str)" = []
    connections : "list(str)" = []
    genTreeFormulas(formulas, descriptions, connections, treeLevels)
    writeSltFile(f"benchmarks/Tree{treeLevels}.slt", descriptions, connections)


Path("benchmarks").mkdir(parents=True, exist_ok=True)

for ll in lineLengths:
    genLineTopology(ll)

for ll in branchLengths:
    genBranchTopology(numBranchs, ll)

for td in treeDepths:
    genTreeTopology(td)
