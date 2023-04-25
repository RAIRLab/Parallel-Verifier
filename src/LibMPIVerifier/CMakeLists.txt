

add_library(LibMPIVerifier STATIC)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(LibMPIVerifier SharedVerifier ${MPI_C_LIBRARIES})

target_sources(LibMPIVerifier PRIVATE
    LibMPIVerifier.hpp
    assumptionUpdates.cpp
    MPIUtil.hpp
    MPIUtil.cpp
    getLayersMPI.cpp
    verifyAlpha.cpp
    verifyLoadBalance.cpp
    verifyNoOpt.cpp
    verifyOriginal.cpp
)