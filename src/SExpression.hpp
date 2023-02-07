
#ifndef SEXPRESSION_H
#define SEXPRESSION_H

#include<string>
#include<vector>
#include<queue>
#include<iostream>

struct sExpression{
    //Possible types
    enum class Type{Keyword = 0, Number = 1, String = 2, Symbol = 3, List = 4};

    //Data
    sExpression::Type type;

    //union {
        std::string value;                 //Only valid if type != List
        std::vector<sExpression> members;  //Only Valid if type == List
    //};

    //Members
    sExpression();                                             //Default constructor
    sExpression(const std::string sExpressionString);        //Parse an S-Expression string into S-expression
    sExpression(const sExpression& toCopy);
    sExpression(const sExpression&& toMove);                   //Move constructor
    ~sExpression();                                            //Deconstructor
    
    sExpression& operator=(const sExpression&& toMove);      //Move assignment
    sExpression& operator=(const sExpression& toCopy);       //Move assignment
    
    std::string toString(bool expand = false) const;
    void print(bool expand = false) const;

    //These only work if type is list
    sExpression& operator[](const size_t index);          //Access values based on indicies
    const sExpression& at(const size_t index) const;

    sExpression& operator[](const std::string&& key);     //Access values based on keywords

    sExpression::Type getTypeAt(const size_t index) const;             
    std::string getValueAt(const size_t index) const;             //returns the value in the sExpression at index if its not a sub list
    unsigned int getNumAt(const size_t) const;                    //same as getValue but if type == num casts it to an int first

    bool contains(const sExpression& t) const;
    sExpression atPosition(std::queue<uid_t> pos) const;
    std::queue<uid_t> positionOf(const sExpression& t) const;
    std::queue<uid_t> positionOf(const sExpression& t, std::queue<uid_t> pos) const;
};

// For equality
bool operator==(const sExpression& s1, const sExpression& s2);
bool operator!=(const sExpression& s1, const sExpression& s2);

//For using sExpressions with std::cout
std::ostream& operator<<(std::ostream& os, const sExpression& object);

//sExpression hashing
namespace std{
    template <>
    struct hash<sExpression>{
        std::size_t operator()(const sExpression& k) const;
    };
}

#endif


