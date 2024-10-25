/* syntaxerror.hpp
PURPOSE:
- A class for syntax errors
*/
#ifndef SYNTAXERROR_HPP
#define SYNTAXERROR_HPP

#include <cstdint>
#include <string>
#include <iostream>

class SyntaxError {
public:
    enum class SyntaxErrorType {
        IncompletePhrase,
        UnknownToken
    };

    SyntaxError(SyntaxErrorType type, uint32_t line, uint32_t pos)
        : type(type), line(line), pos(pos) {}

    SyntaxErrorType getType() {
        return type;
    }

    uint32_t getLine() {
        return line;
    }

    uint32_t getPos() {
        return pos;
    }

    void printMessage(std::ostream& os);

private:
    SyntaxErrorType type;
    uint32_t line, pos;
};

#endif // SYNTAXERROR_HPP

inline void SyntaxError::printMessage(std::ostream& os) {
    os << "Error: ";
    switch(type) {
    case SyntaxError::SyntaxErrorType::IncompletePhrase:
        os << "Incomplete phrase";
        break;
    case SyntaxError::SyntaxErrorType::UnknownToken:
        os << "Cannot parse token";
        break;
    }
    os << " on Line " << line << ":" << pos << ".\n";
}