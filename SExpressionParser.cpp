
#include<cctype>
#include<vector>
#include<string>
#include<unordered_set>

#include<fstream>
#include<iostream>

#include "SExpressionParser.h"

//Token constructor
Token::Token(TokenType type_, std::string value_)
:type(type_), value(value_)
{}

//Lexer Code ============================================================================

//Lexer Globals
const std::unordered_set<char> ingnoreChars({' ', '\t', '\n'});                 //Ignored characters when parsing 
const std::unordered_set<char> endingChars({' ', '(', ')', '\t', '\n'});        //Tokens that end a previous token if encountered


//Lexer helper functions ================================================================

//starting at position i of sExpressionString, adds a token with all chars untill a " is hit
inline void addStringToken(const std::string& expressionString, int &i, std::vector<Token>& tokens){
    bool foundEndOfString = false;
    std::string tokenValue = "";
    int j;
    for(j = i+1; j<expressionString.length(); j++){
        char strCur = expressionString[j];
        char strLast = expressionString[j-1];
        if(strCur == '\"' && strLast != '\\'){  //if we're the end of the string, but not an escaped end of string
            foundEndOfString = true;
            break;
        }else{
            tokenValue += strCur;
        }
    }
    if(foundEndOfString)
        tokens.emplace_back(TokenType::String, tokenValue);
    else
        throw std::runtime_error("s-expression lexer error: no matching \" found for \" in position " + std::to_string(i));
    i = j;
}

inline void addKeyToken(const std::string& expressionString, int &i, std::vector<Token>& tokens){
    std::string tokenValue = "";
    int j;
    for(j = i+1; j<expressionString.length(); j++){
        char curKeyChar = expressionString[j];
        if(endingChars.find(curKeyChar) != endingChars.end())
            break;
        else
            tokenValue += curKeyChar;
    }
    tokens.emplace_back(TokenType::Keyword, tokenValue);
    i = j-1;
}

inline void addNormalToken(const std::string& expressionString, int &i, std::vector<Token>& tokens){
    bool numeric = true;
    std::string tokenValue = "";
    int j;
    for(j = i; j < expressionString.length(); j++){
        char strCur = expressionString[j];
        if(endingChars.find(strCur) != endingChars.end()){
            break;
        }else{
            numeric = numeric && isdigit(strCur);
            tokenValue += strCur;
        }
    }
    if(numeric)
        tokens.emplace_back(TokenType::Number, tokenValue);
    else
        tokens.emplace_back(TokenType::Symbol, tokenValue);
    i = j-1;
}


//Main Lexer ============================================================================

std::vector<Token> lex(const std::string& expressionString){
    std::vector<Token> tokens; 
    for(int i = 0; i < expressionString.length(); i++){
        char curChar = expressionString[i];
        if(curChar == '('){
            tokens.emplace_back(TokenType::Left_Parenthesis, "(");
        }else if(curChar == ')'){
            tokens.emplace_back(TokenType::Right_Parenthesis, ")");
        }else if(curChar == '\"'){
            addStringToken(expressionString, i, tokens);
        }else if(curChar == ':'){                                      
            addKeyToken(expressionString, i, tokens);
        }else if(ingnoreChars.find(curChar) == ingnoreChars.end()){  //Symbols and Numbers
            addNormalToken(expressionString, i, tokens);
        }
    }
    return tokens;
}




//Testing 
/*
std::string readFile(const std::string&& path){
    std::ifstream file(path);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

int main(){
    std::string s = readFile("laryLikesLucy.slt");
    std::vector<Token> t = lex(s);
    for(Token& tok : t){
        std::cout<<tok.type<<":"<<tok.value<<"\n";
    }
    return 0;
}

*/

