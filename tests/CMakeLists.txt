


# SExpressions ----------------------------------------------------------------

add_executable(SExpressionTest)
target_link_libraries(SExpressionTest SExpression)
target_sources(SExpressionTest PRIVATE 
    SExpressionTest.cpp
)

add_test(NAME SExpressionSingle COMMAND SExpressionTest "A")
add_test(NAME SExpressionSimple  COMMAND SExpressionTest "(or A B)")
add_test(NAME SExpressionTestCompound COMMAND SExpressionTest 
"(if (or A B) (and A (not (iff A C))))")

# ProofIO ---------------------------------------------------------------------

add_executable(ProofIOTest)
target_link_libraries(ProofIOTest Proof ProofIO SExpression)
target_sources(ProofIOTest PRIVATE
    ProofIOTest.cpp
)

add_test(
    NAME ProofIOHyperslateFirstOrderJustifications 
    COMMAND ProofIOTest "proofs/hyperslate/proofs/AllFirstOrderJusts.slt"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_test(
    NAME ProofIOLazyslateAllPropositionalRules
    COMMAND ProofIOTest "proofs/lazyslate/allRulesProof.json"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Serial Verifier -------------------------------------------------------------------


add_test(
    NAME SerialVerifierFavorite
    COMMAND SerialVerifier "proofs/lazyslate/ourFavorite.json"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(SerialVerifierFavorite PROPERTIES TIMEOUT 5) 

add_test(
    NAME SerialVerifierAllPropositionalRules
    COMMAND SerialVerifier "proofs/lazyslate/allRulesProof.json"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(SerialVerifierAllPropositionalRules PROPERTIES TIMEOUT 5) 

add_test(
    NAME SerialVerifierOrDistro
    COMMAND SerialVerifier "proofs/lazyslate/orDistribution.json"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(SerialVerifierOrDistro PROPERTIES TIMEOUT 5) 


# Parallel Verifier -------------------------------------------------------------------

add_test(
    NAME ParallelVerifierFavorite
    COMMAND mpirun -N 4 ParallelVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(ParallelVerifierFavorite PROPERTIES TIMEOUT 5) 

add_test(
    NAME ParallelVerifierAllPropositionalRules
    COMMAND mpirun -N 4 ParallelVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(ParallelVerifierAllPropositionalRules PROPERTIES TIMEOUT 5) 

add_test(
    NAME ParallelVerifierOrDistro
    COMMAND mpirun -N 4 ./ParallelVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(ParallelVerifierOrDistro PROPERTIES TIMEOUT 5) 