/* intermediatenode.cpp
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/
#include "intermediatenode.h"
#include <cstdint>

void IntermediateNode::generateTree(std::vector<std::tuple<std::string, uint32_t, uint32_t>> tokens) {
    if (token.getType() != Token::TokenType::UNSET) destroy();

    IntermediateNode *lastTopLevel = nullptr;
    IntermediateNode *last = nullptr;

    for (std::tuple<std::string, uint32_t, uint32_t> tuple : tokens) {
        std::string value = std::get<0>(tuple);
        uint32_t line = std::get<1>(tuple);
        uint32_t pos = std::get<2>(tuple);
        bool first = true, inLink = false, inHtml = false, inArg = false;
        if (last != nullptr) {
            if (last->token.getType() == Token::TokenType::KEYWORD &&
                    (last->token.getValue() == "open" ||
                    last->token.getValue() == "file"))
                inLink = true;
            else if (last->getParent() != nullptr &&
                    last->getParent()->token.getType() == Token::TokenType::KEYWORD &&
                    (last->getParent()->token.getValue() == "open" ||
                    last->getParent()->token.getValue() == "file"))
                inLink = true;
            if (Token::getPhraseLength(last->token) > last->getNumberChildren())
                first = last->getNumberChildren() > 0;
            else if (last->getParent() != nullptr)
                first = Token::getPhraseLength(last->getParent()->token) <= last->getParent()->getNumberChildren();
            if (last->token.getType() == Token::TokenType::KEYWORD &&
                    last->token.getValue() == "create")
                inHtml = true;
            if (last->token.getType() == Token::TokenType::NAME &&
                    last->getParent() != nullptr &&
                    last->getParent()->token.getType() == Token::TokenType::KEYWORD &&
                    last->getParent()->token.getValue() == "create")
                inArg = true;
        }
        Token cToken = Token(Token::getLiteral(value, inLink), value, line,
                pos, first, inLink, inHtml, inArg);

        // The first token
        if (last == nullptr) {
            last = this;
            lastTopLevel = this;
            token = cToken;
            continue;
        }

        // Assignments
        if (cToken.getType() == Token::TokenType::ASSIGNMENT) {
            // If the last element is not part of a const or argument list automatically assume equality
            if (last->getParent() == nullptr ||
                    (last->getParent()->token.getType() != Token::TokenType::CONST &&
                    last->getParent()->token.getType() != Token::TokenType::ARGUMENT_LIST))
                cToken = Token(Token::TokenType::BINARY_OPERATOR, "=",
                        cToken.getLine(), cToken.getPos());
            // Double equals will always be equality (and technically any higher number of equals in a row though that's undefined behavior)
            if (last->getParent() != nullptr && last->getParent()->getNumberChildren() == 1 &&
                    (last->getParent()->token.getType() == Token::TokenType::BINARY_OPERATOR ||
                    last->getParent()->token.getType() == Token::TokenType::ASSIGNMENT) &&
                    last->getParent()->token.getValue() == "=") {
                last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "=",
                        last->getParent()->token.getLine(), last->getParent()->token.getPos());
                continue; // Just need to adjust and move on since it's not a new token
            }

            // Need to replace last node and then have it as a child
            IntermediateNode *newNode = new IntermediateNode();
            newNode->token = cToken;
            if (last->previous != nullptr) {
                if (last->hasParent) last->previous->firstChild = newNode;
                else last->previous->nextSibling = newNode;
            } if (last->nextSibling != nullptr) last->nextSibling->previous = newNode;
            newNode->hasParent = last->hasParent;
            newNode->previous = last->previous;
            newNode->firstChild = last;
            newNode->nextSibling = last->nextSibling;
            last->hasParent = true;
            last->previous = newNode;
            last->nextSibling = nullptr;

            continue;
        }

        // Binary Operators

        // Unary Operators

        // TODO: I am here
    }
}

std::vector<SyntaxError> getErrors() {

}

void IntermediateNode::addSibling(IntermediateNode* node) {
    if (nextSibling != nullptr) nextSibling->addSibling(node);
    else {
        nextSibling = node;
        node->hasParent = false;
        node->previous = this;
    }
}

void IntermediateNode::addChild(IntermediateNode* node) {
    if (firstChild != nullptr) firstChild->addSibling(node);
    else {
        firstChild = node;
        node->hasParent = true;
        node->previous = this;
    }
}

IntermediateNode * IntermediateNode::getParent() {
    if (previous == nullptr) return nullptr;
    if (hasParent) return previous;
    return previous->getParent();
}

IntermediateNode * IntermediateNode::getChild(uint32_t index) {
    if (index == 0) return firstChild;
    if (firstChild == nullptr) return nullptr;
    return firstChild->getSibling(index-1);
}

IntermediateNode * IntermediateNode::getSibling(uint32_t index) {
    if (index == 0) return nextSibling;
    if (nextSibling == nullptr) return nullptr;
    return nextSibling->getSibling(index-1);
}

uint32_t IntermediateNode::getNumberChildren() {
    if (firstChild == nullptr) return 0;
    return 1 + firstChild->getNumberYoungerSiblings();
}

uint32_t IntermediateNode::getNumberYoungerSiblings() {
    if (nextSibling == nullptr) return 0;
    return 1 + nextSibling->getNumberYoungerSiblings();
}

IntermediateNode * IntermediateNode::operator[](uint32_t index) {
    return getChild(index);
}

IntermediateNode::~IntermediateNode() {
    destroy();
}

void IntermediateNode::destroy() {
    token = Token();
    hasParent = false;
    previous = nullptr;
    if (firstChild != nullptr) firstChild->destroy();
    if (nextSibling != nullptr) nextSibling->destroy();
    delete firstChild;
    delete nextSibling;
}