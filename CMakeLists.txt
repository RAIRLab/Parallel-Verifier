
cmake_minimum_required(VERSION 3.12)
project("Parallel Verifier" VERSION 2.0.0)

# Config ======================================================================

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fstack-protector -s")


# Packages ====================================================================

find_package(MPI REQUIRED)


find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
#message("JsonCPP:" ${JSONCPP_LIBRARIES} ${JSONCPP_INCLUDE_DIRS})
include_directories(${JSONCPP_INCLUDE_DIRS})

# Executables =================================================================

add_executable(SerialVerifier)
target_link_libraries(SerialVerifier ${JSONCPP_LIBRARIES})

add_executable(ParallelVerifier)
target_link_libraries(ParallelVerifier MPI::MPI_CXX)
target_link_libraries(ParallelVerifier ${JSONCPP_LIBRARIES})

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

set(PROOF_IO
    src/ProofIO/HyperSlate.hpp
    src/ProofIO/HyperSlate.cpp
    src/ProofIO/Lazyslate.hpp
    src/ProofIO/Lazyslate_shared.cpp
    src/ProofIO/Lazyslate_jsoncpp.cpp
    src/ProofIO/ProofIO.hpp
    src/ProofIO/ProofIO.cpp
)

set(SHARED_VERIFIER_SOURCES
    ${PROOF_IO}
    ${INFERENCE_RULES}
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

# Testing =====================================================================

enable_testing()

# SExpressions ----------------------------------------------------------------

add_executable(SExpressionTest)
target_link_libraries(SExpressionTest ${JSONCPP_LIBRARIES})
target_sources(SExpressionTest PRIVATE 
    src/SExpression.hpp
    src/SExpression.cpp
    tests/SexpressionTest.cpp
)

add_test(NAME SExpressionTest1 COMMAND SExpressionTest "A")
add_test(NAME SExpressionTest2 COMMAND SExpressionTest "(or A B)")
add_test(NAME SExpressionTest3 COMMAND SExpressionTest 
    "(if (or A B) (and A (not (iff A C))))"
)

# ProofIO ---------------------------------------------------------------------

add_executable(ProofIOTest)
target_link_libraries(ProofIOTest ${JSONCPP_LIBRARIES})
target_sources(ProofIOTest PRIVATE
    ${PROOF_IO}
    src/SExpression.cpp
    src/SExpression.hpp
    src/Proof.hpp
    src/Proof.cpp
    tests/ProofIOTest.cpp
)

add_test(
    NAME "ProofIOTest1" 
    COMMAND ProofIOTest "proofs/hyperslate/proofs/AllFirstOrderJusts.slt"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(
    NAME "ProofIOTest2"
    COMMAND ProofIOTest "proofs/lazyslate/allRulesProof.json"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Serial Verifier -------------------------------------------------------------------