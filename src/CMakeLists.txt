add_subdirectory(LibOMPVerifier)
add_subdirectory(LibParallelVerifier)
add_subdirectory(SExpression)
add_subdirectory(Proof)
add_subdirectory(ProofIO)
add_subdirectory(SharedVerifier)

add_executable(SerialVerifier)
target_link_libraries(SerialVerifier SharedVerifier)
target_sources(SerialVerifier PRIVATE SerialVerifier.cpp)

add_executable(ParallelVerifier)
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(ParallelVerifier MPI::MPI_CXX LibParallelVerifier SharedVerifier)
target_sources(ParallelVerifier PRIVATE ParallelVerifier.cpp)

add_executable(OMPVerifier)
target_link_libraries(OMPVerifier LibOMPVerifier SharedVerifier)
target_sources(OMPVerifier PRIVATE OMPVerify.cpp)

add_executable(ProofConv)
target_link_libraries(ProofConv ProofIO)
target_sources(ProofConv PRIVATE ProofConv.cpp)