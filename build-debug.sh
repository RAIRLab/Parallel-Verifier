


# Cmake wrapper script that Automatically runs all the CMake Build
# and copies our executables into bin

set -e
SCRIPTDIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
mkdir -p $SCRIPTDIR/build
cd $SCRIPTDIR/build
cmake ../ -D CMAKE_BUILD_TYPE=Debug
make
cd $SCRIPTDIR