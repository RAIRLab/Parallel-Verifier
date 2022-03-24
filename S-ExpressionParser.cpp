
#include<vector>
#include<string>


enum TokenType{
    SExpression,
    Symbol,         //
    Keyword,        //Items begining with :
    Integer,        //Numerics
    String,         //Items enclosed in quotes
}

struct SExpression{
    std::vector<SExpression> children;

};

struct Token{
    TokenType type;
    std::string value;
};

std::vector<Token> lex(std::string sExpressionString){

}