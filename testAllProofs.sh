
#Tests the serial vs parallel implementation on all proofs in the proofs directory
#Does not test on the benchmarks

PROOFFILES=$(find proofs -type f)

for file in $PROOFFILES
do
    echo $file
    ./serialVerif.exe $file
    mpirun -n 5 parallelVerif.exe $file
    echo
done
