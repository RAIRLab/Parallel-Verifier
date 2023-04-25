
import os
import subprocess
import matplotlib
import matplotlib.pyplot as plt

#Quick bench Paramaters 

#Proof to quickbench
proof = "../proofs/hyperslate/benchmarks/Tree18.slt"
ranks = [1, 2, 3, 4, 5, 6, 7, 8] #Num Ranks to test
#ParallelVerifier Args to test with
MPIArgs = [
#    ("Original", "Serial"),
#    ("NoOpt", "Serial"),
#    ("LoadBalance", "Serial")
]

OMPArgs = [
    "OG",
    "LB",
    "BF"
]


def extractCycles(s):
    return int(str(s).split()[2])

#Serial Results
serialRawOut = subprocess.check_output(['../build/bin/SerialVerifier', proof])
serialCycles = extractCycles(serialRawOut)
print("Serial Cycles", serialCycles)
plt.plot(ranks, [serialCycles] * len(ranks), label="Serial")

#OMP Parallel Results
my_env = os.environ.copy()
for method in OMPArgs:
    cycles = []
    for rank in ranks:
        my_env["OMP_NUM_THREADS"] = str(rank) #Set OpneMP threads
        rawOutput = subprocess.check_output(['../build/bin/OMPVerifier', proof, method], env=my_env)
        cycle = extractCycles(rawOutput)
        cycles.append(cycle)
        print("OMP", method, rank, "Cycles", cycle)
    plt.plot(ranks, cycles, label=f"OMP: {method}")

#MPI Parallel Results
for arg in MPIArgs:
    cycles = []
    for rank in ranks:
        rawOutput = subprocess.check_output(['mpirun', '-N', str(rank), '../build/bin/ParallelVerifier', proof] + list(arg))
        cycle = extractCycles(rawOutput)
        cycles.append(cycle)
        print("MPI", str(arg), "Rank", rank, "Cycles", cycle)
    plt.plot(ranks, cycles, label=f"MPI: {str(arg)}")

plt.yscale('log')
plt.title('growth')
plt.xlabel('Ranks')
plt.ylabel('Cycles')
plt.legend()
plt.savefig("res.png")
