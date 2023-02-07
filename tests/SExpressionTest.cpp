

#include <iostream>
#include <cassert>

#include "../src/SExpression.hpp"

int main(int argc, char** argv){
    assert(argc == 2);
    sExpression expression(argv[1]);
    std::cout<<expression.toString();
    return 0;
}