//Testing 

#include<fstream>
#include<iostream>

#include"SExpression.hpp"

std::string readFile(const std::string&& path){
    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main(){
    std::string sExpStr = readFile("laryLikesLucy.slt");
    //int i = 2;
    //std::vector<Token> t = lex(sExpStr);
    //std::vector<Token> st = grabSubExpression(t, i);
    sExpression sExp(sExpStr);
    sExpression sExp2(sExp.toString());
    sExp2.print();
    return 0;
}