
add_library(SharedVerifier STATIC)

target_link_libraries(SharedVerifier SExpression ProofIO)

set(INFERENCE_RULES
    InferenceRules/rules.hpp
    InferenceRules/and.cpp
    InferenceRules/assume.cpp
    InferenceRules/equals.cpp
    InferenceRules/exists.cpp
    InferenceRules/forall.cpp
    InferenceRules/if.cpp
    InferenceRules/iff.cpp
    InferenceRules/not.cpp
    InferenceRules/or.cpp
)

target_sources(SharedVerifier PRIVATE
    ${INFERENCE_RULES}
    SharedVerifier.hpp
    SharedVerifier.cpp
    Substitution.hpp
    Substitution.cpp
    timing.hpp
)


