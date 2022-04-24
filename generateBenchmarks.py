
#James Oswald 

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
def genLineFormulas(formulas : "list(str)", depth: int):
    if depth == 0:
        formulas.append("A")
    else:
        genLineFormulas(formulas, depth-1)
        formulas.append(f"(or A {formulas[-1]})")

lineFormulas : "list(str)" = []
genLineFormulas(lineFormulas, 100)
descriptions : "list(str)" = []
connections : "list(str)" = []
for i, formula in enumerate(lineFormulas):
    if i == 0:
        descriptions.append(f"(:X 0 :Y 0 :ID {i} :NAME \"{i}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::ASSUME)")
    if i != 0:
        connections.append(f"(:CONCLUSION {i} :PREMISES ({i-1}))")
        descriptions.append(f"(:X 0 :Y 0 :ID {i} :NAME \"{i}\" :FORMULA \"{formula}\" :JUSTIFICATION LOGIC::OR-INTRO)")
writeSltFile("benchmarks/Strait.slt", descriptions, connections)

branhOne : "list(str)" = []
branhTwo : "list(str)" = []
genLineFormulas(lineFormulas, 100)