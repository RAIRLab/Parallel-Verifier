
#Automatically runs all the CMake Build and copies our executables into bin

set -e
SCRIPTDIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
mkdir -p $SCRIPTDIR/build
cd $SCRIPTDIR/build
cmake ../
make
cd $SCRIPTDIR