
#include"../SharedVerifier.hpp"
#include"../../Proof/Proof.hpp"

#define FORWARD_DECLARE_INTRO_ELIM(N)\
bool verify##N##ElimSyntax(const Proof& p, const VertId vertexId);\
bool verify##N##ElimSemantics(const Proof& p, const VertId vertexId, Assumptions& assumptions);\
bool verify##N##IntroSyntax(const Proof& p, const VertId vertexId);\
bool verify##N##IntroSemantics(const Proof& p, const VertId vertexId, Assumptions& assumptions);\

bool verifyAssumptionSyntax(const Proof& p, const VertId vertexId);
bool verifyAssumptionSemantics(const Proof& p, const VertId vertexId, Assumptions& assumptions);
FORWARD_DECLARE_INTRO_ELIM(And);
FORWARD_DECLARE_INTRO_ELIM(Or);
FORWARD_DECLARE_INTRO_ELIM(Not);
FORWARD_DECLARE_INTRO_ELIM(If);
FORWARD_DECLARE_INTRO_ELIM(Iff);