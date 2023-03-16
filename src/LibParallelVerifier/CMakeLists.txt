
add_library(LibParallelVerifier STATIC)

find_package(MPI REQUIRED)
target_link_libraries(LibParallelVerifier SharedVerifier MPI::MPI_CXX)

target_sources(LibParallelVerifier PRIVATE
    LibParallelVerifier.hpp
    LibParallelVerifier.cpp
    MPIUtil.hpp
    MPIUtil.cpp
    getLayersMPI.cpp
    getLayersSerial.cpp
    verifyLoadBalance.cpp
)