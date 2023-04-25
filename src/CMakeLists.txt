add_subdirectory(LibOMPVerifier)
add_subdirectory(LibMPIVerifier)
add_subdirectory(SExpression)
add_subdirectory(Proof)
add_subdirectory(ProofIO)
add_subdirectory(SharedVerifier)

add_executable(SerialVerifier)
target_link_libraries(SerialVerifier SharedVerifier)
target_sources(SerialVerifier PRIVATE SerialVerifier.cpp)

add_executable(MPIVerifier)
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(MPIVerifier MPI::MPI_CXX LibMPIVerifier SharedVerifier)
target_sources(MPIVerifier PRIVATE MPIVerifier.cpp)

add_executable(OMPVerifier)
target_link_libraries(OMPVerifier LibOMPVerifier SharedVerifier)
target_sources(OMPVerifier PRIVATE OMPVerifier.cpp)

add_executable(ProofConv)
target_link_libraries(ProofConv ProofIO)
target_sources(ProofConv PRIVATE ProofConv.cpp)