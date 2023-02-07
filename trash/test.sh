

PROOFFILES=$(find tests -type f)

echo -e FileName SerialOut SerialTime ParallelOut ParallelTime > /tmp/testRes
for file in $PROOFFILES
do
    serialResults=( $(./bin/serialVerif.exe $file) )
    parallelResults=( $(mpirun -n 5 ./bin/parallelVerif.exe $file) )
    echo -e $file ${serialResults[5]} ${serialResults[0]} ${parallelResults[5]} ${parallelResults[0]} >> /tmp/testRes
done
column -t -s " " < /tmp/testRes
rm /tmp/testRes