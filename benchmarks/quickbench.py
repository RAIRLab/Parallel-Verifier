
import os
import subprocess
import matplotlib
import matplotlib.pyplot as plt

#Quick bench Paramaters 

#Proof to quickbench
proof = "../proofs/hyperslate/benchmarks/Tree14.slt"
ranks = [1, 2, 3, 4, 5, 6] #Num Ranks to test
#ParallelVerifier Args to test with
args = [
    ("Original", "Serial"),
    ("NoOpt", "Serial"),
    ("LoadBalance", "Serial")
]

def extractCycles(s):
    return int(str(s).split()[2])

#Serial Results
serialRawOut = subprocess.check_output(['../build/bin/SerialVerifier', proof])
serialCycles = extractCycles(serialRawOut)
print("Serial Cycles", serialCycles)
plt.plot(ranks, [serialCycles] * len(ranks), label="Serial")

#OMP Parallel Results
cycles = []
my_env = os.environ.copy()
for rank in ranks:
    my_env["OMP_NUM_THREADS"] = str(rank) #Set OpneMP threads
    rawOutput = subprocess.check_output(['../build/bin/OMPVerifier', proof], env=my_env)
    cycle = extractCycles(rawOutput)
    cycles.append(cycle)
    print("OMP Rank", rank, "Cycles", cycle)
plt.plot(ranks, cycles, label="OMP")

#MPI Parallel Results
for arg in args:
    cycles = []
    for rank in ranks:
        rawOutput = subprocess.check_output(['mpirun', '-N', str(rank), '../build/bin/ParallelVerifier', proof] + list(arg))
        cycle = extractCycles(rawOutput)
        cycles.append(cycle)
        print(str(arg), "Rank", rank, "Cycles", cycle)
    plt.plot(ranks, cycles, label=str(arg))

plt.yscale('log')
plt.title('growth')
plt.xlabel('Ranks')
plt.ylabel('Cycles')
plt.legend()
plt.savefig("res.png")