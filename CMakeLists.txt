
cmake_minimum_required(VERSION 3.12)
project("Parallel Verifier" VERSION 2.0.0)

# Config ======================================================================

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
cmake_policy(SET CMP0076 NEW)

# Executables =================================================================

add_executable(SerialVerifier)
add_executable(ParallelVerifier)

# Packages ====================================================================

find_package(MPI REQUIRED)
target_link_libraries(ParallelVerifier MPI::MPI_CXX)

# Sources =====================================================================

set(INFERENCE_RULES
    src/InferenceRules/and.hpp
    src/InferenceRules/assume.hpp
    src/InferenceRules/equals.hpp
    src/InferenceRules/exists.hpp
    src/InferenceRules/forall.hpp
    src/InferenceRules/if.hpp
    src/InferenceRules/iff.hpp
    src/InferenceRules/not.hpp
    src/InferenceRules/or.hpp
)

set(SHARED_VERIFIER_SOURCES
    ${INFERENCE_RULES}
    src/hyperslateUtils.cpp
    src/hyperslateUtils.hpp
    src/Proof.cpp
    src/Proof.hpp
    src/SExpression.cpp
    src/SExpression.hpp
    src/SharedVerifier.cpp
    src/SharedVerifier.hpp
    src/Substitution.cpp
    src/Substitution.hpp
)

target_sources(SerialVerifier PRIVATE
    ${SHARED_VERIFIER_SOURCES}
    src/SerialVerifier.cpp
)

target_sources(ParallelVerifier PRIVATE
    ${SHARED_VERIFIER_SOURCES}
    src/ParallelVerifier.cpp
)

