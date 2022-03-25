
#include<string>
#include<vector>

enum TokenType{
    Symbol,             //Any other ID
    Keyword,            //Items begining with :
    Number,             //Unsigned Ints
    String,             //Items enclosed in quotes
    Left_Parenthesis,
    Right_Parenthesis
};

struct Token{
    TokenType type;
    std::string value;
    Token(TokenType type_, std::string value_);
};

