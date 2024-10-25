/* token.hpp
PURPOSE:
- Token object for building the token tree, and related helper functions
- This class does not deal with verification or parents/children, just the individual token
*/
#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "defines.h"
#include "notimplementedexception.hpp"
#include <string>
#include <cstdint>

class Token {
public:
    /*
    Full Phrase Token Types (top-level phrases must be these):
    - const
    - any value expression

    Value Expression Token Types:
    - keyword (opening up a phrase)
    - any pure value expression

    Pure Value Expression Token Types (not really used):
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
    - const
    - assignment
    - any operator, the operator is the parent and the operand(s) the children
    - list literal, one of the two that can have any number of children, stores value expressions,
        the last comma is stored as filler to make sure syntax isn't broken, the closing bracket
        just removes the last comma so its known as done, a filler comma is initially inserted because
        of this way to check if its finished
    - argument list, the other of the two, works similar but has assignments not value expressions

    */
    enum class TokenType {
        UNSET = 0,
        CONST,
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
        UNARY_OPERATOR, // All unary operators are prefix
        BINARY_OPERATOR,
        ASSIGNMENT,
        UNKNOWN
    };

    Token();
    Token(Token &token);
    Token(TokenType literalType, const std::string& value, const uint32_t& line,
        const uint32_t& pos, bool first, bool inLink, bool inHtml);
    Token(TokenType type, const std::string& value, const uint32_t& line,
        const uint32_t& pos);

    TokenType getType() const;
    const std::string& getValue() const;
    void setValue(const std::string& value);
    const uint32_t& getLine() const;
    const uint32_t& getPos() const;

    static uint32_t getPhraseLength(Token kw);
    static bool doesAcceptInPosition(Token kw, Token t, uint32_t pos, bool final=false);
    static bool isPureValueExpression(Token t);
    static bool isValueExpression(Token t);
    static bool isFullPhrase(Token t);
    static bool isPhrase(Token t);
    static TokenType getLiteral(std::string token, bool inLink=false);

private:
    TokenType type;
    std::string value;
    uint32_t line, pos;
};

inline Token::Token() 
    : type(Token::TokenType::UNSET), value(""), line(0), pos(0) {}

inline Token::Token(Token &token)
    : type(token.type), value(token.value), line(token.line), pos(token.pos) {}

// A more complete string -> token built on top of the type from getLiteral()
inline Token::Token(TokenType literalType, const std::string& value, const uint32_t& line,
        const uint32_t& pos, bool first, bool inLink, bool inHtml) {
    type = literalType;
    this->value = value;
    this->line = line;
    this->pos = pos;
    switch (literalType) {
        case Token::TokenType::NAME:
            if(inHtml) type = Token::TokenType::HTMLPART;
            break;
        case Token::TokenType::STRING_LITERAL:
            this->value = value.substr(1, value.size()-1);
            break;
        case Token::TokenType::COLOR_LITERAL:
            this->value = value.substr(1);
            break;
        case Token::TokenType::UNKNOWN:
            if (value.size() == 1) switch (value[0]) {
                case '+':
                case '-':
                    type = first ? Token::TokenType::UNARY_OPERATOR :
                        Token::TokenType::BINARY_OPERATOR;
                    break;
                case '/':
                    type = (first && inLink) ? Token::TokenType::UNARY_OPERATOR :
                        Token::TokenType::BINARY_OPERATOR;
                    break;
                case '*':
                case '%':
                case '&':
                case '|':
                case '^':
                case '≥':
                case '≤':
                case '≠':
                case '≈':
                    type = Token::TokenType::BINARY_OPERATOR;
                    break;
                case '!':
                    this->value = "not";
                case '~':
                    type = Token::TokenType::UNARY_OPERATOR;
                    break;
                case str2int("xor"):
                case str2int("and"):
                case str2int("or"):
                    type = Token::TokenType::BINARY_OPERATOR;
                    break;
                case str2int("not"):
                    type = Token::TokenType::UNARY_OPERATOR;
                    break;
                case '=':
                    type = Token::TokenType::ASSIGNMENT;
                    break;
                case '[':
                    type = Token::TokenType::LIST_LITERAL;
                    break;
                case '(':
                    type = Token::TokenType::UNARY_OPERATOR;
                    // brackets used for math can be treated as a unary operator that does nothing,
                    // argument lists aren't detected here
                    break;
                case ']':
                case ')':
                    type = Token::TokenType::FILLER;
                    break;
                }
            break;
    }
}

inline Token::Token(TokenType type, const std::string& value, const uint32_t& line, const uint32_t& pos) 
    : type(type), value(value), line(line), pos(pos) {}

inline Token::TokenType Token::getType() const {
    return type;
}

inline const std::string& Token::getValue() const {
    return value;
}

inline void Token::setValue(const std::string& value) {
    this->value = value;
}

inline const uint32_t& Token::getLine() const {
    return line;
}

inline const uint32_t& Token::getPos() const {
    return pos;
}

uint32_t Token::getPhraseLength(Token kw) {
    switch (kw.type) {
        case Token::TokenType::KEYWORD:
            switch (str2int(kw.value.c_str())) {
                case str2int("create"):
                    return 1; // Create takes in an htmlpart or binary '('
                case str2int("open"):
                case str2int("file"):
                    return 1; // Open & File takes in a file literal
                case str2int("colorset"):
                    return 3; // Color set takes in 3 assignments
                case str2int("foreach"):
                    return 5; // For each takes in a variable name, then a filler in word, then a value expression,
                                        // then a filler do keyword, then a full phrase
                case str2int("using"):
                    return 5; // Using takes in a value expression, then a filler as word, then a variable name,
                                        // then a filler do keyword, then a full phrase
                case str2int("export"):
                #ifdef Ver0_1_0
                case str2int("output"):
                #endif
                    return 1; // Export & Output takes in a value expression
                default:
                    return 0;
            }
        case Token::TokenType::ASSIGNMENT:
            return 2;
        case Token::TokenType::BINARY_OPERATOR:
            return 2;
        case Token::TokenType::UNARY_OPERATOR:
            return 1;
        case Token::TokenType::CONST:
            return 1;
        case Token::TokenType::ARGUMENT_LIST:
            return -1;  // -1 (the uint32_t max) represents variable length
        case Token::TokenType::LIST_LITERAL:
            return -1;
        default:
            return 0;
    }
}

bool Token::doesAcceptInPosition(Token kw, Token t, uint32_t pos, bool final=false) {
    // Some stuff changes during construction, like assignments are built up with the name first,
                                // so there is a difference between final and not
    switch (kw.type) {
    case Token::TokenType::KEYWORD:
            switch (str2int(kw.value.c_str())) {
                case str2int("create"):
                    // Create takes in an htmlpart or binary '('
                    if (pos == 0) return t.type == Token::TokenType::HTMLPART ||
                        (t.type == Token::TokenType::BINARY_OPERATOR && t.value == "(");
                    return false;
                case str2int("open"):
                case str2int("file"):
                    // Open & File takes in a file literal
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
                    switch (pos) {
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
                    switch (pos) {
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
                #ifdef Ver0_1_0
                case str2int("output"):
                #endif
                    // Export & Output takes in a value expression
                    return isValueExpression(t) && pos == 0;
                default:
                    return 0;
            }
        case Token::TokenType::ASSIGNMENT:
            if (pos == 1) return isValueExpression(t);
            if (pos == 0) return t.type == Token::TokenType::NAME;
            return false;
        case Token::TokenType::BINARY_OPERATOR:
            if (kw.value == "(") {
                if (pos == 1) return t.type == Token::TokenType::ARGUMENT_LIST;
                if (pos == 0) return isValueExpression(t) || t.type == Token::TokenType::HTMLPART;
                return false;
            }
            return isValueExpression(t) && (pos == 0 || pos == 1);
        case Token::TokenType::UNARY_OPERATOR:
            return isValueExpression(t) && pos == 0;
        case Token::TokenType::CONST:
            if (pos != 0) return false;
            if (t.type == Token::TokenType::ASSIGNMENT) return true;
            if (t.type == Token::TokenType::NAME) return !final;
            return false;
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

bool Token::isPureValueExpression(Token t) {
    switch (t.type) {
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

bool Token::isValueExpression(Token t) {
    return isPureValueExpression(t) || (t.type == Token::TokenType::KEYWORD);
}

bool Token::isFullPhrase(Token t) {
    return isValueExpression(t) || (t.type == Token::TokenType::CONST);
}

bool Token::isPhrase(Token t) {
    switch(t.type) {
        case Token::TokenType::CONST:
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

// Not full type resolution! Just the following:
// Const, StringLiteral, BoolLiteral, ThisLiteral, ColorLiteral,
// NumericLiteral, Keyword, Filler words, Name (possibly), or Unknown
// If expecting a file literal it will include .'s and give
// FileLiteral instead of Name, but it won't do that elsewise
Token::TokenType Token::getLiteral(std::string token, bool inLink=false) {
    if (token.front() == '"' && token.back() == '"')
        return Token::TokenType::STRING_LITERAL;
    switch (str2int(token.c_str())) {
        case str2int("true"):
        case str2int("false"):
            return Token::TokenType::BOOL_LITERAL;
        case str2int("this"):
            return Token::TokenType::THIS_LITERAL;
        case str2int("const"):
            return Token::TokenType::CONST;
        case str2int("create"):
        case str2int("open"):
        case str2int("file"):
        case str2int("colorset"):
        case str2int("foreach"):
        case str2int("using"):
        case str2int("export"):
        #ifdef Ver0_1_0
        case str2int("output"):
        #endif
            return Token::TokenType::KEYWORD;
        case str2int("as"):
        case str2int("in"):
        case str2int("do"):
        case str2int(","):
            return Token::TokenType::FILLER;
        case str2int("xor"):
        case str2int("and"):
        case str2int("or"):
        case str2int("not"):
            return Token::TokenType::UNKNOWN;
    };

    bool alphanumeric = true, numeric = true, hex = true, first = true;
    const bool hashtag = token.front() == '#';
    for (char c : token) {
        if(first && hashtag) {
            first = false;
            continue;
        }
        // Numbers can be made of 0-9, .
        if (!isdigit(c) && c != '.')
            numeric = false;
        // Hex can be made of 0-9, a-f, A-F
        if (!isxdigit(c))
            hex = false;
        // Words can be made of 0-9, a-z, A-Z, _, inLink: .
        if (!isalnum(c) && c != '_' && (!inLink || c != '.'))
            alphanumeric = false; 
    }
    if (hashtag)
        return hex ? Token::TokenType::COLOR_LITERAL : Token::TokenType::UNKNOWN;
    if (numeric)
        return Token::TokenType::NUMERIC_LITERAL;
    if (alphanumeric)
        return inLink ? Token::TokenType::FILE_LITERAL : Token::TokenType::NAME;
    return Token::TokenType::UNKNOWN;
}

// From https://stackoverflow.com/questions/16388510/evaluate-a-string-with-a-switch-in-c
// Credit to @Serhiy
constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

#endif // TOKEN_HPP
