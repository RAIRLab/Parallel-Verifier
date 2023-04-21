
add_library(LibOMPVerifier STATIC)

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED) #IhatempiIhateMPIIhateMPI

target_link_libraries(LibOMPVerifier
    LibParallelVerifier
    OpenMP::OpenMP_CXX
    SharedVerifier
    MPI::MPI_CXX
)

target_sources(LibOMPVerifier PRIVATE
    LibOMPVerifier.hpp
    verifyOpenMP.cpp
)