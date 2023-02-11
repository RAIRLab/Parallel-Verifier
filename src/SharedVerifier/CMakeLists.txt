
add_library(SharedVerifier STATIC)

target_link_libraries(SharedVerifier SExpression ProofIO)

set(INFERENCE_RULES
    InferenceRules/and.hpp
    InferenceRules/assume.hpp
    InferenceRules/equals.hpp
    InferenceRules/exists.hpp
    InferenceRules/forall.hpp
    InferenceRules/if.hpp
    InferenceRules/iff.hpp
    InferenceRules/not.hpp
    InferenceRules/or.hpp
)

target_sources(SharedVerifier PRIVATE
    ${INFERENCE_RULES}
    SharedVerifier.hpp
    SharedVerifier.cpp
    Substitution.hpp
    Substitution.cpp
    timing.hpp
)


