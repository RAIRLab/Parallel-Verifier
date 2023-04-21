
import subprocess
import matplotlib
import matplotlib.pyplot as plt

proof = "../proofs/hyperslate/benchmarks/Tree12.slt"
ranks = [1, 2, 4, 6]

args = [
    ("Alpha", "OzSerial"),
    ("NoOpt", "OzSerial"),
    ("LoadBalance", "OzSerial")
]

def extractCycles(s):
    return int(str(s).split()[2])

#Serial Results
serialRawOut = subprocess.check_output(['../build/bin/SerialVerifier', proof])
serialCycles = extractCycles(serialRawOut)
print("Serial Cycles", serialCycles)
plt.plot(ranks, [serialCycles] * len(ranks), label="Serial")

#Parallel Results
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