/**
 * @file SExpression.cpp
 * @author James Oswald (jamesoswald111@gmail.com, oswalj@rpi.edu)
 * @date 3/27/22
 * @brief The implementation of SExpression class
 * 
 * This file implements the sExpression class outlined in SExpression.hpp 
 * along with various other utils that make this possible such: as a combine
 * S-Expression lexer and tokenizer, a parser, and a token class.
 * 
 * At some point I attempted to document this file, I did not finish, sorry
 * -James of 4/18/22
 */

#include<cctype>
#include<vector>
#include<string>
#include<unordered_set>

#include "SExpression.hpp"

// Token Helper Class ===================================================================

/**
 * @brief Enum containing possible token types
 * Some members of this Enum correspond directly to sExpression::Type members values
 * and are directly converted to them within parserBaseCase(). This must be kept in
 * mind in the event of future modification. 
 */
enum class TokenType{
    Keyword = 0,            ///< Tokens beginning with :
    Number = 1,             ///< Tokens containing unsigned Ints (At the moment other numeric types are unsupported by the lexer)
    String = 2,             ///< Tokens enclosed in quotes
    Symbol = 3,             ///< Tokens containing any other valid identifier or name
    Left_Parenthesis = 4,   ///< A token containing a left parenthesis "("
    Right_Parenthesis = 5   ///< A token containing a right parenthesis ")"
};

struct Token{
    TokenType type;
    std::string value;
    Token(TokenType type_, std::string value_);
};

Token::Token(TokenType type_, std::string value_)
:type(type_), value(value_)
{}

//Tokenizer and Lexer Code ============================================================================
/**
 * @name The Lexer
 */
///@{
    
//Lexer Globals
const std::unordered_set<char> ignoreChars({' ', '\t', '\n'});            //Ignored characters when parsing 
const std::unordered_set<char> endingChars({' ', '(', ')', '\t', '\n'});   //Tokens that end a previous token if encountered

//Lexer helper functions

//starting at position i of sExpressionString, adds a token with all chars until a " is hit
inline void addStringToken(const std::string& expressionString, int &i, std::vector<Token>& tokens){
    bool foundEndOfString = false;
    std::string tokenValue = "";
    size_t j;
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
        throw std::runtime_error(
            "S-Expression lexer error: no matching \" found\
             for \" in position " + std::to_string(i));
    i = j;
}

inline void addKeyToken(const std::string& expressionString, int &i, std::vector<Token>& tokens){
    std::string tokenValue = "";
    size_t j;
    //Grab chars for our token until we hit an end character or reach the end of the expression string
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
    bool numeric = true;    //Keep track of weather or not the token is a numeric (an unsigned int)
    std::string tokenValue = "";
    size_t j;
    //Grab chars for our token until we hit an end character or reach the end of the expression string
    for(j = i; j < expressionString.length(); j++){  
        char strCur = expressionString[j];
        if(endingChars.find(strCur) != endingChars.end()){ //if we hit an ending char
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


/**
 * @brief Takes an S-expression string and tokenizes it, as well as lexing it by identifying the tokens
 * @param expressionString A string containing an s-expression to tokenize
 * @return A vector containing the list of token objects containing both their content and type.
 * @throws std::runtime-error if an error is encountered lexing tokens 
 * The lexer both tokenizes and lexes the expressionString. The lexer can fail while lexing a token if the token
 * is not well formed, I.E. a string token without a closing quote.
 */
std::vector<Token> lex(const std::string& expressionString){
    std::vector<Token> tokens; 
    for(int i = 0; static_cast<size_t>(i) < expressionString.length(); i++){
        char curChar = expressionString[i];
        if(curChar == '('){
            tokens.emplace_back(TokenType::Left_Parenthesis, "(");
        }else if(curChar == ')'){
            tokens.emplace_back(TokenType::Right_Parenthesis, ")");
        }else if(curChar == '\"'){
            addStringToken(expressionString, i, tokens);
        }else if(curChar == ':'){                                      
            addKeyToken(expressionString, i, tokens);
        }else if(ignoreChars.find(curChar) == ignoreChars.end()){  //Both Symbols and Numbers
            addNormalToken(expressionString, i, tokens);
        }
    }
    return tokens;
}
///@}
//Parser Code ========================================================================
/**
 * @name The Parser
 */
///@{


/**
 *   @brief A helper for the parser
 *   @param tokens the list of tokens to grab the sub expression from
 *   @param i the token index to begin grabbing the sub expression at
 *   @pre i must be the index of a left parenthesis expression. 
 *   Starting at position i, will grab a subexpression from the token array and
 *   move i to the position after the matching parenthesis. 
 *   ```
 *   For example:
 *       tokens = ( ...  (a (b c)) ...)
 *                       ^        ^
 *                       i        Ni
 *       then grabSubExpression returns (a (b c)) and changes i to position Ni 
 *   ```
 */
inline std::vector<Token> grabSubExpression(const std::vector<Token>& tokens, int& i){
    int parenthesisCounter = 1;
    bool foundMatch = false;
    size_t j;
    for(j = i+1; j < tokens.size()-1; j++){ //find the index of the closing parenthesis and store it in j
        if(tokens[j].type == TokenType::Right_Parenthesis && parenthesisCounter == 1){
            foundMatch = true;
            break;
        }else if(tokens[j].type == TokenType::Right_Parenthesis){
            parenthesisCounter--;
        } else if(tokens[j].type == TokenType::Left_Parenthesis){
            parenthesisCounter++;
        }
    }
    if(foundMatch){
        std::vector<Token> subExpressionTokens(tokens.begin() + i, tokens.begin() + j + 1);
        i = j;
        return subExpressionTokens;
    }else{
        throw std::runtime_error("S-Expression parsing error: could not find matching parenthesis");
    }
}


inline void parserBaseCase(const std::vector<Token>& tokens, sExpression& expression){
    Token token = tokens[0];
    //Make sure the token type is also a valid s-expression type
    if(!(token.type >= TokenType::Keyword && token.type <= TokenType::Symbol)) 
        throw std::runtime_error("S-Expression parsing error: invalid token type for conversion "
            + std::to_string((int)token.type));
    expression.type = static_cast<sExpression::Type>(token.type);
    expression.value = token.value;
}

sExpression parseTokens(const std::vector<Token>& tokens);

inline void parserRecursiveCase(const std::vector<Token>& tokens, sExpression& expression){
    if(tokens[0].type != TokenType::Left_Parenthesis 
            || tokens[tokens.size()-1].type != TokenType::Right_Parenthesis)
        throw std::runtime_error("S-Expression parsing error: Expected list to be wrapped by Parenthesis");
    expression.type = sExpression::Type::List;
    expression.members = std::vector<sExpression>();
    std::vector<Token> subTokens;
    for(int i = 1; static_cast<size_t>(i) < tokens.size()-1; i++){
        if(tokens[i].type == TokenType::Left_Parenthesis){
            subTokens = grabSubExpression(tokens, i);
        }else{
            subTokens.clear();
            subTokens.push_back(tokens[i]);
        }
        sExpression child = parseTokens(subTokens);
        expression.members.push_back(child);
    }
}

// Parser ===============================================================================

//Recursive s-expression parser
sExpression parseTokens(const std::vector<Token>& tokens){
    sExpression expression; 
    if(tokens.size() == 0){
        throw std::runtime_error("S-Expression parsing error: Nothing to parse");
    }else if(tokens.size() == 1){       
        parserBaseCase(tokens, expression); //Base case, only passing a list with a single token
    }else{                                  
        parserRecursiveCase(tokens, expression); //Recursive case, passing token list
    }
    return expression;
}

//sExpression members ==================================================================

sExpression::sExpression()
:type(sExpression::Type::Keyword), value(""){
}

//Initialize this s-expression object from an s-expression string
sExpression::sExpression(const std::string sExpressionString){
    std::vector<Token> tokens = lex(sExpressionString);
    *this = parseTokens(tokens);
}

sExpression::sExpression(const sExpression&& expression){
    *this = std::move(expression);
}

sExpression::sExpression(const sExpression& expression){
    *this = expression;
}

sExpression::~sExpression(){
}

sExpression& sExpression::operator=(const sExpression&& expression){
    type = expression.type;
    if(expression.type == sExpression::Type::List)
        members = std::move(expression.members);
    else
        value = std::move(expression.value);
    return *this;
}

sExpression& sExpression::operator=(const sExpression& expression){
    type = expression.type;
    if(expression.type == sExpression::Type::List)
        members = expression.members;
    else
        value = expression.value;
    return *this;
}

//Convert an S expression to a string representation
std::string recursiveToString(const sExpression& expression, size_t depth, bool expand){
    std::string result = "";
    if(expression.type == sExpression::Type::List){
        result += (expand ? std::string(' ', depth+1) + "(\n" : "(");
        for(const sExpression& subExpression : expression.members)
            result += recursiveToString(subExpression, depth+1, expand);
        result.pop_back(); //removes the extra space
        result += (expand ? std::string(' ', depth=1) + ")\n" : ")");
    }else if(expression.type == sExpression::Type::Keyword){
        result += (expand ? std::to_string((int)expression.type) + std::string(' ', depth)\
               + ":" + expression.value + "\n" : ":" + expression.value + " ");
    }else{
        result += (expand ? std::to_string((int)expression.type) + std::string(' ', depth)\
               + expression.value + "\n" : expression.value + " ");
    }
    return result;
} 

std::string sExpression::toString(bool expand) const{
    return recursiveToString(*this, 0, expand);
}

void sExpression::print(bool expand) const{
    std::string stringExpression = recursiveToString(*this, 0, expand);
    std::cout << stringExpression << std::endl;
}

std::ostream& operator<<(std::ostream& os, const sExpression& expression){
    os<<recursiveToString(expression, 0, false);
    return os;
}

sExpression& sExpression::operator[](const size_t index){
    if(type != sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: can not index a non-list type S-Expression");
    return members[index];
}

const sExpression& sExpression::at(const size_t index) const{
    if(type != sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: can not index a non-list type S-Expression");
    return members[index];
}

/*
    Returns a reference to the sExpression following a key in the current sExpression
*/
sExpression& sExpression::operator[](const std::string&& key){
    if(type != sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: Can not lookup key in a non-list type S-Expression");
    for(size_t i = 0; i < members.size(); i++)
        if(members[i].type == sExpression::Type::Keyword && members[i].value == key){
            if(i == members.size()-1)
                throw std::runtime_error("S-Expression Error: Key found at end of list without pair");
            if(members[i+1].type == sExpression::Type::Keyword)
                throw std::runtime_error("S-Expression Error: Key value is another key");
            return members[i+1];
        }
    throw std::runtime_error("S-Expression Error: Key value not found");
}

sExpression::Type sExpression::getTypeAt(const size_t index) const{
    if(type != sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: can not index a non-list type S-Expression");
    return members[index].type;
}             

std::string sExpression::getValueAt(const size_t index) const{
    if(type != sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: can not index a non-list type S-Expression");
    if(members[index].type == sExpression::Type::List)
        throw std::runtime_error("S-Expression Error: index " + std::to_string(index) + " contains a list type S-expression");
    return members[index].value;
}   

unsigned int sExpression::getNumAt(const size_t index) const{
    if(members[index].type != sExpression::Type::Number)
        throw std::runtime_error("S-Expression Error: item at index " + std::to_string(index) + " is not a number");
    return std::stoi(getValueAt(index));
}

bool operator!=(const sExpression& s1, const sExpression& s2) {
    return !(s1 == s2);
}

bool operator==(const sExpression& s1, const sExpression& s2) {
    if (s1.type != s2.type) { return false; }
    if (s1.type != sExpression::Type::List) { return s1.value == s2.value; }

    // Assume its a list
    if (s1.members.size() != s2.members.size()) {
        return false;
    }

    // Make sure each item in the list are equal
    for (size_t i = 0; i < s1.members.size(); i++) {
        if (s1.members[i] != s2.members[i]) {
            return false;
        }
    }
    return true;
}

bool sExpression::contains(const sExpression& t) const {
    if (type != sExpression::Type::List) { return false; }

    bool within = false;
    for (size_t i = 0; i < members.size(); i++) {
        within |= (members[i] == t); if (within) { break; }
        within |= members[i].contains(t); if (within) { break; }
    }

    return within;
}

// Finds the first position that matches the sub-term t
std::queue<uid_t> sExpression::positionOf(const sExpression& t) const {
    return this->positionOf(t, std::queue<uid_t>());
}

// Finds the first position that matches the sub-term t
std::queue<uid_t> sExpression::positionOf(const sExpression& t, std::queue<uid_t> pos) const {
    if (*this == t) {
        return pos;
    }

    if (this->type != sExpression::Type::List) {
        throw std::out_of_range("S-Expression Error: Sub-term is not within term");
    }

    for (size_t i = 0; i < this->members.size(); i++) {
        try {
            std::queue<uid_t> newPos = pos;
            pos.push(i);
            std::queue<uid_t> result = this->members[i].positionOf(t, newPos);
            return result;
        } catch (...) { }
    }

    throw std::out_of_range("S-Expression Error: Sub-term is not within term");
}

// Returns the sub-term at the specified position
// [] is the whole term
// Each index in the pos variable represents the position of the argument
// starting at 0.
sExpression sExpression::atPosition(std::queue<uid_t> pos) const {
    if (pos.size() == 0) {
        sExpression result = *this;
        return result;
    }

    if (this->type != sExpression::Type::List) {
        throw std::out_of_range("S-Expression Error: Position does not exist for this term");
    }

    const uid_t pos_id = pos.front();
    pos.pop();
    if (pos_id >= this->members.size()) {
        throw std::out_of_range("S-Expression Error: Position does not exist for this term");
    }

    return this->atPosition(pos);
}

std::size_t std::hash<sExpression>::operator()(const sExpression &expr) const{
    return std::hash<std::string>()(expr.toString(false));
}


