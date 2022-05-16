
#Strong Scaling benchmark

BENCHMARKFILES=$(find benchmarks -type f)

if [ $(uname -m) == "ppc64le" ]; #if we're on AiMos
then
    RANKS=(1 2 4 8 16 24 32)
else
    RANKS=(1 2 3 4 5 6)
fi

for file in $BENCHMARKFILES
do 
    echo $file : 
    echo Serial
    ./serialVerif.exe $file
    for numRanks in ${RANKS[@]}
    do
        echo Parallel $numRanks
        mpirun --bind-to core -np $numRanks parallelVerif.exe $file
    done
    echo
done