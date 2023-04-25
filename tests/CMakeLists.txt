
include(ProcessorCount)
ProcessorCount(NumProcessors)
message(
    "Num CPUs for MPI Testing (Note tests may not be accuracte if not enough CPU cores are present): "
    ${NumProcessors} " Cores"
)

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
set_tests_properties(SerialVerifierAllPropositionalRules PROPERTIES TIMEOUT 10) 

add_test(
    NAME SerialVerifierOrDistro
    COMMAND SerialVerifier "proofs/lazyslate/orDistribution.json"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(SerialVerifierOrDistro PROPERTIES TIMEOUT 10) 

# OMP Verifier ----------------------------------------------------------------

add_test(
    NAME OMPVerifierFavorite
    COMMAND OMPVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(OMPVerifierFavorite PROPERTIES TIMEOUT 10) 

add_test(
    NAME OMPVerifierAllPropositionalRules
    COMMAND OMPVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(OMPVerifierAllPropositionalRules PROPERTIES TIMEOUT 10) 

add_test(
    NAME OMPVerifierOrDistro
    COMMAND OMPVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(OMPVerifierOrDistro PROPERTIES TIMEOUT 10)

# Parallel Verifier -------------------------------------------------------------------

# NoOpt MPI

add_test(
    NAME MPIVerifierFavorite_NoOpt_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierFavorite_NoOpt_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierAllPropositionalRules_NoOpt_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierAllPropositionalRules_NoOpt_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierOrDistro_NoOpt_MPI
    COMMAND mpirun -N ${NumProcessors} ./MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierOrDistro_NoOpt_MPI PROPERTIES TIMEOUT 10) 

# NoOpt Serial

add_test(
    NAME MPIVerifierFavorite_NoOpt_Serial
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json NoOpt Serial
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierFavorite_NoOpt_Serial PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierAllPropositionalRules_NoOpt_Serial
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json NoOpt Serial
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierAllPropositionalRules_NoOpt_Serial PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierOrDistro_NoOpt_Serial
    COMMAND mpirun -N ${NumProcessors} ./MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json NoOpt Serial
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierOrDistro_NoOpt_Serial PROPERTIES TIMEOUT 10)

#LoadBalance MPI

add_test(
    NAME MPIVerifierFavorite_LoadBalance_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json LoadBalance MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierFavorite_LoadBalance_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierAllPropositionalRules_LoadBalance_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json LoadBalance MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierAllPropositionalRules_LoadBalance_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierOrDistro_LoadBalance_MPI
    COMMAND mpirun -N ${NumProcessors} ./MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json LoadBalance MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierOrDistro_LoadBalance_MPI PROPERTIES TIMEOUT 10)

#Original MPI

add_test(
    NAME MPIVerifierFavorite_Original_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/ourFavorite.json Original MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierFavorite_Original_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierAllPropositionalRules_Original_MPI
    COMMAND mpirun -N ${NumProcessors} MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/allRulesProof.json Original MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierAllPropositionalRules_Original_MPI PROPERTIES TIMEOUT 10) 

add_test(
    NAME MPIVerifierOrDistro_Original_MPI
    COMMAND mpirun -N ${NumProcessors} ./MPIVerifier ${CMAKE_SOURCE_DIR}/proofs/lazyslate/orDistribution.json Original MPI
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_tests_properties(MPIVerifierOrDistro_Original_MPI PROPERTIES TIMEOUT 10)