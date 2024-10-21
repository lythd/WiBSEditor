/* token.hpp
PURPOSE:
- Token object for building the token tree, and related helper functions
- This class does not deal with verification or parents/children, just the individual token
*/
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "notimplementedexception.hpp"
#include <string>

class Token {
public:
    /*
    Full Phrase Token Types (top-level phrases must be these):
    - const keyword
    - any value expression

    Value Expression Token Types:
    - keyword (opening up a phrase) except const
    - name (constant/variable)
    - string literal
    - bool literal
    - numeric_literal
    - this_literal
    - color_literal
    - list_literal
    - unary_operator
    - binary_operator

    All Phrases (phrases are anything with children):
    - any keyword
    - assignment
    - any operator, the operator is the parent and the operand(s) the children
    - list literal, one of the two that can have any number of children, stores value expressions,
        the last comma is stored as filler to make sure syntax isn't broken, the closing bracket
        just removes the last comma so its known as done, a filler comma is initially inserted because
        of this way to check if its finished
    - argument list, the other of the two, works similar but has assignments not value expressions

    */
    enum class TokenType {
        KEYWORD,
        FILLER, // Just used during construction to ensure syntax, but ignored in execution
        NAME,
        HTMLPART,
        STRING_LITERAL, // Quote marks are removed upon construction
        BOOL_LITERAL,
        NUMERIC_LITERAL,
        THIS_LITERAL, // The value is undefined because it is not needed
        FILE_LITERAL,
        COLOR_LITERAL, // Hashtag are removed upon construction
        LIST_LITERAL,
        ARGUMENT_LIST,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        ASSIGNMENT,
        UNKNOWN
    };

    Token(TokenType type, const std::string& value);

    TokenType getType() const;
    const std::string& getValue() const;

    static uint getPhraseLength(Token kw);
    static bool doesAcceptInPosition(Token kw, Token t, uint pos, bool final=false);
    static bool Token::isValueExpression(Token t);
    static bool Token::isFullPhrase(Token t);
    static bool Token::isPhrase(Token t);

private:
    TokenType type;
    std::string value;
};

inline Token::Token(TokenType type, const std::string& value) 
    : type(type), value(value) {}

inline Token::TokenType Token::getType() const {
    return type;
}

inline const std::string& Token::getValue() const {
    return value;
}

uint Token::getPhraseLength(Token kw) {
    switch(kw.type) {
    case Token::TokenType::KEYWORD:
        switch(str2int(kw.value.c_str())) {
        case str2int("const"):
            return 1; // Const takes in an assigment
        case str2int("create"):
            return 2; // Create takes in an htmlpart and argument list
        case str2int("open"):
            return 1; // Open takes in a file literal
        case str2int("file"):
            return 1; // File takes in a file literal
        case str2int("colorset"):
            return 3; // Color set takes in 3 assignments
        case str2int("foreach"):
            return 5; // For each takes in a variable name, then a filler in word, then a value expression,
                                // then a filler do keyword, then a full phrase
        case str2int("using"):
            return 5; // Using takes in a value expression, then a filler as word, then a variable name,
                                // then a filler do keyword, then a full phrase
        case str2int("export"):
            return 1; // Export takes in a value expression
        default:
            return 0;
        }
    case Token::TokenType::ASSIGNMENT:
        return 2;
    case Token::TokenType::BINARY_OPERATOR:
        return 2;
    case Token::TokenType::UNARY_OPERATOR:
        return 1;
    case Token::TokenType::ARGUMENT_LIST:
        return -1;  // -1 (the uint max) represents variable length
    case Token::TokenType::LIST_LITERAL:
        return -1;
    default:
        return 0;
    }
}

bool Token::doesAcceptInPosition(Token kw, Token t, uint pos, bool final=false) {
    // Some stuff changes during construction, like assignments are built up with the name first,
                                // so there is a difference between final and not
    switch(kw.type) {
    case Token::TokenType::KEYWORD:
        switch(str2int(kw.value.c_str())) {
        case str2int("const"):
            // Const takes in an assigment
            if (pos != 0) return false;
            if (t.type == Token::TokenType::ASSIGNMENT) return true;
            if (t.type == Token::TokenType::NAME) return !final;
            return false;
        case str2int("create"):
            // Create takes in an htmlpart and argument list
            if (pos == 1) return t.type == Token::TokenType::ARGUMENT_LIST;
            if (pos == 0) return t.type == Token::TokenType::HTMLPART;
            return false;
        case str2int("open"):
            // Open takes in a file literal
            return t.type == Token::TokenType::FILE_LITERAL && pos == 0;
        case str2int("file"):
            // File takes in a file literal
            return t.type == Token::TokenType::FILE_LITERAL && pos == 0;
        case str2int("colorset"):
            // Color set takes in 3 assignments
            if (pos < 0 || pos > 2) return false;
            if (t.type == Token::TokenType::ASSIGNMENT) return true;
            if (t.type == Token::TokenType::NAME) return !final;
            return false;
        case str2int("foreach"):
            // For each takes in a variable name, then a filler in word, then a value expression,
                                // then a filler do keyword, then a full phrase
            switch(pos) {
            case 0:
                return t.type == Token::TokenType::NAME;
            case 1:
                return t.type == Token::TokenType::FILLER && t.value == "in";
            case 2:
                return isValueExpression(t);
            case 3:
                return t.type == Token::TokenType::FILLER && t.value == "do";
            case 4:
                return isFullPhrase(t);
            }
        case str2int("using"):
            // Using takes in a value expression, then a filler as word, then a variable name,
                                // then a filler do keyword, then a full phrase
            switch(pos) {
            case 0:
                return isValueExpression(t);
            case 1:
                return t.type == Token::TokenType::FILLER && t.value == "as";
            case 2:
                return t.type == Token::TokenType::NAME;
            case 3:
                return t.type == Token::TokenType::FILLER && t.value == "do";
            case 4:
                return isFullPhrase(t);
            }
        case str2int("export"):
            // Export takes in a value expression
            return isValueExpression(t) && pos == 0;
        default:
            return 0;
        }
    case Token::TokenType::ASSIGNMENT:
        if (pos == 1) return isValueExpression(t);
        if (pos == 0) return t.type == Token::TokenType::NAME;
        return false;
    case Token::TokenType::BINARY_OPERATOR:
        return isValueExpression(t) && (pos == 0 || pos == 1);
    case Token::TokenType::UNARY_OPERATOR:
        return isValueExpression(t) && pos == 0;
    case Token::TokenType::ARGUMENT_LIST:
        if (t.type == Token::TokenType::ASSIGNMENT) return true;
        if (t.type == Token::TokenType::NAME) return !final;
        if (t.type == Token::TokenType::FILLER) return t.value == ",";
    case Token::TokenType::LIST_LITERAL:
        if (isValueExpression(t)) return true;
        if (t.type == Token::TokenType::NAME) return !final;
        if (t.type == Token::TokenType::FILLER) return t.value == ",";
    default:
        return false;
    }
}

bool Token::isValueExpression(Token t) {
    switch(t.type) {
    case Token::TokenType::KEYWORD:
        return t.value != "const";
    case Token::TokenType::NAME:
    case Token::TokenType::STRING_LITERAL:
    case Token::TokenType::BOOL_LITERAL:
    case Token::TokenType::NUMERIC_LITERAL:
    case Token::TokenType::THIS_LITERAL:
    case Token::TokenType::COLOR_LITERAL:
    case Token::TokenType::LIST_LITERAL:
    case Token::TokenType::UNARY_OPERATOR:
    case Token::TokenType::BINARY_OPERATOR:
        return true;
    default:
        return false;
    }
}

bool Token::isFullPhrase(Token t) {
    return isValueExpression(t) || (t.type == Token::TokenType::KEYWORD && t.value == "const");
}

bool Token::isPhrase(Token t) {
    switch(t.type) {
    case Token::TokenType::KEYWORD:
    case Token::TokenType::ASSIGNMENT:
    case Token::TokenType::ARGUMENT_LIST:
    case Token::TokenType::LIST_LITERAL:
    case Token::TokenType::UNARY_OPERATOR:
    case Token::TokenType::BINARY_OPERATOR:
        return true;
    default:
        return false;
    }
}

// From https://stackoverflow.com/questions/16388510/evaluate-a-string-with-a-switch-in-c
// Credit to @Serhiy
constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

#endif // TOKEN_HPP
