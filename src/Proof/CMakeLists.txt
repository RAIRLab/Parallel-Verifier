
add_library(Proof STATIC)

target_link_libraries(Proof SExpression ProofIO)

target_sources(Proof PRIVATE
    Proof.hpp
    Proof.cpp
)