/* tokenparser.cpp
PURPOSE:
- Converts raw text into digestable tokens for compilation
*/
#include "tokenparser.h"
#include <sstream>

TokenParser::TokenParser() {}

std::vector<std::string> TokenParser::parse(const std::string& text) {
    tokenize(text);
    return tokens;
}

std::vector<std::string> TokenParser::getTokens() const {
    return tokens;
}

void TokenParser::tokenize(const std::string& text) {
    tokens.clear();
    std::string currentToken;
    bool inString = false;
    bool inComment = false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char currentChar = text[i];

        // Check if we are inside a comment
        if (inComment) {
            if (currentChar == '\n') inComment = false;
            continue;
        }

        // Check if we are inside a string
        if (inString) {
            currentToken += currentChar;
            if (currentChar == '"') {
                inString = false;
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            continue;
        }

        // Check for the start of a comment
        if (currentChar == '/' && i>0 && text[i-1] == '/') {
            // We check the second one, so that the last token being cleared is already handled for us by the first slash
            inComment = true;
            tokens.pop_back(); // There would have been a '/' added otherwise from the first one
            continue;
        }

        // Check for the start of a string
        if (currentChar == '"') {
            inString = true;
            currentToken += currentChar; // Include the starting quote
            continue;
        }

        // Check if the character is part of a word (letters, digits, underscore), the first is allowed to be a hashtag for color literals
        if (std::isalnum(currentChar) || currentChar == '_' || (currentToken.empty() && currentChar == '#'))
            currentToken += currentChar;
        else {
            // If we have a current token, push it to tokens
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear(); // Reset current token
            }

            // If the character is not whitespace, add it as a standalone symbol token
            if (!std::isspace(currentChar)) tokens.push_back(std::string(1, currentChar));
        }
    }

    // Add the last token if it exists
    if (!currentToken.empty()) tokens.push_back(currentToken);
}