
# Run the debugger
ROOTDIR="../$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
mpirun -n 2 xterm -hold -e gdb -ex run --args $ROOTDIR/build/bin/ParallelVerifier $ROOTDIR/proofs/lazyslate/ourFavorite.json
