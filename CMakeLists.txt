
cmake_minimum_required(VERSION 3.12)
project("Parallel Verifier" VERSION 2.0.0)

# Config ======================================================================

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fstack-protector -s")

# Executables =================================================================

add_executable(SerialVerifier)
add_executable(ParallelVerifier)

add_executable(ProofIOTests)

# Packages ====================================================================

find_package(MPI REQUIRED)
target_link_libraries(ParallelVerifier MPI::MPI_CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
message("JsonCPP:" ${JSONCPP_LIBRARIES})
target_link_libraries(ParallelVerifier ${JSONCPP_LIBRARIES})
target_link_libraries(ParallelVerifier ${JSONCPP_LIBRARIES})
target_link_libraries(SerialVerifier ${JSONCPP_LIBRARIES})
target_link_libraries(ProofIOTests ${JSONCPP_LIBRARIES})


#find_package(jsoncpp REQUIRED)
#target_link_libraries(ParallelVerifier jsoncpp_lib)
#target_link_libraries(SerialVerifier jsoncpp_lib)
#target_link_libraries(ProofIOTests jsoncpp_lib)
#message("JsonCPP:" ${jsoncpp_lib})

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

#target_sources(ProofConverter PRIVATE
#    src/ProofConverter.cpp
#)

target_sources(ProofIOTests PRIVATE
    ${PROOF_IO}
    src/SExpression.cpp
    src/SExpression.hpp
    src/Proof.hpp
    src/Proof.cpp
    tests/ProofIOTests.cpp
)

# Testing =====================================================================

enable_testing()

add_test(
    NAME ProofIOTest
    COMMAND $<TARGET_FILE:ProofIOTests>
)