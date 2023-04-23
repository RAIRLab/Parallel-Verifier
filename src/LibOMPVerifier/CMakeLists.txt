
add_library(LibOMPVerifier STATIC)

find_package(OpenMP REQUIRED)

target_link_libraries(LibOMPVerifier PRIVATE SharedVerifier ${OpenMP_CXX_FLAGS})
target_compile_options(LibOMPVerifier PRIVATE ${OpenMP_CXX_FLAGS})

target_sources(LibOMPVerifier PRIVATE
    LibOMPVerifier.hpp
    verifyOpenMP.cpp
)