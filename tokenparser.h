/* tokenparser.h
PURPOSE:
- Converts raw text into digestable tokens for compilation
*/
#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <vector>
#include <string>

class TokenParser {
public:
    TokenParser();
    std::vector<std::string> parse(const std::string& text);
    std::vector<std::string> getTokens() const;

private:
    std::vector<std::string> tokens;
    void tokenize(const std::string& text);
};

#endif // TOKENPARSER_H
