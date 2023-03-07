
add_subdirectory(SExpression)
add_subdirectory(Proof)
add_subdirectory(ProofIO)
add_subdirectory(SharedVerifier)

add_executable(SerialVerifier)
target_link_libraries(SerialVerifier SharedVerifier)
target_sources(SerialVerifier PRIVATE SerialVerifier.cpp)

add_executable(ParallelVerifier)
find_package(MPI REQUIRED)
target_link_libraries(ParallelVerifier SharedVerifier MPI::MPI_CXX)
target_sources(ParallelVerifier PRIVATE ParallelVerifier.cpp)