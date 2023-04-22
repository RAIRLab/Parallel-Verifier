
# Cmake wrapper script that Automatically runs all the CMake Build
# and copies our executables into bin

set -e
ROOTDIR="../$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
mkdir -p $ROOTDIR/build
cd $ROOTDIR/build
cmake ../
make
cd $ROOTDIR