

#Uncomment to enable all print statements for debugging
#add_compile_definitions(PRINT_DEBUG)


add_library(LibParallelVerifier STATIC)

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(LibParallelVerifier SharedVerifier ${MPI_C_LIBRARIES})

target_sources(LibParallelVerifier PRIVATE
    LibParallelVerifier.hpp
    assumptionUpdates.cpp
    MPIUtil.hpp
    MPIUtil.cpp
    getLayersMPI.cpp
    verifyAlpha.cpp
    verifyLoadBalance.cpp
    verifyNoOpt.cpp
    verifyOriginal.cpp
)