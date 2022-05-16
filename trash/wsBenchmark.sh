

BENCHMARKFILES=$(find benchmarks -type f)

for file in $BENCHMARKFILES
do 
    echo $file : 
    echo Serial
    ./serialVerif.exe $file
    echo Parallel 
    mpirun --bind-to core -np 4 parallelVerif.exe $file
    echo
done