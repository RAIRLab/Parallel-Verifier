
add_library(ProofIO STATIC)

#Link against JsonCPP
#message("JsonCPP:" ${JSONCPP_LIBRARIES} ${JSONCPP_INCLUDE_DIRS})
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})
target_link_libraries(ProofIO ${JSONCPP_LIBRARIES})
#target_link_libraries(ProofIO stdc++fs)

target_link_libraries(ProofIO Proof SExpression)

target_sources(ProofIO PRIVATE
    HyperSlate.hpp
    HyperSlate.cpp
    Lazyslate.hpp
    Lazyslate_shared.cpp
    Lazyslate_jsoncpp.cpp
    ProofIO.hpp
    ProofIO.cpp
)