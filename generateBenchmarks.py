
#James Oswald 


#Hyperparamters ======================================================

#Line Proof Topology
lineLength: int = 500 #Number of nodes in the the linear benchmark

#Branch Proof Topology
numBranchs: int = 12
branchLength: int = 100

#Tree proof Topology
treeDepth: int = 12  #The tree will have 2^treeDepth nodes

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
writeSltFile("benchmarks/Strait.slt", descriptions, connections)


#Generate Branch Topology Proofs
symbols = ["A" + str(i) for i in range(numBranchs)]
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
writeSltFile("benchmarks/Branch.slt", descriptions, connections)

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
        leftChildIndex = genTreeFormulas(formulas, descriptions, connections, treeDepth-1)
        rightChildIndex = genTreeFormulas(formulas, descriptions, connections, treeDepth-1)
        formulas[myIndex] = f"(and {formulas[leftChildIndex]} {formulas[rightChildIndex]})"
        descriptions[myIndex] = f"(:X 0 :Y 0 :ID {myIndex} :NAME \"{myIndex}\" :FORMULA \"{formulas[myIndex]}\" :JUSTIFICATION LOGIC::AND-INTRO)"
        structures.append(f"(:CONCLUSION {myIndex} :PREMISES ({leftChildIndex} {rightChildIndex}))")
        return myIndex

formulas : "list(str)" = []
descriptions : "list(str)" = []
connections : "list(str)" = []
genTreeFormulas(formulas, descriptions, connections, treeDepth)
writeSltFile("benchmarks/Tree.slt", descriptions, connections)