/* tokenparser.h
PURPOSE:
- Converts raw text into digestable tokens for compilation
*/
#ifndef TOKENPARSER_H
#define TOKENPARSER_H

#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

class TokenParser {
public:
    TokenParser();
    std::vector<std::tuple<std::string, uint32_t, uint32_t>> parse(const std::string& text);
    std::vector<std::tuple<std::string, uint32_t, uint32_t>> getTokens() const;

private:
    std::vector<std::tuple<std::string, uint32_t, uint32_t>> tokens;
    void tokenize(const std::string& text);
};

#endif // TOKENPARSER_H
