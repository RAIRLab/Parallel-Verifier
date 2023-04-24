
add_library(LibOMPVerifier STATIC)

find_package(OpenMP REQUIRED)

target_link_libraries(LibOMPVerifier
    OpenMP::OpenMP_CXX
    SharedVerifier
)

target_sources(LibOMPVerifier PRIVATE
    LibOMPVerifier.hpp
    verifyOpenMPOriginal.cpp
    verifyOpenMPLB.cpp
)