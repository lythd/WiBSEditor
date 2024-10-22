/* intermediatenode.h
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/
#ifndef INTERMEDIATENODE_HPP
#define INTERMEDIATENODE_HPP

#include "syntaxerror.hpp"
#include "token.hpp"
#include <vector>
#include <string>
#include <cstdint>

class IntermediateNode {
public:
    void generateTree(std::vector<std::tuple<std::string, uint32_t, uint32_t>> tokens);
    std::vector<SyntaxError> getErrors();
    bool isComplete();
    void addSibling(IntermediateNode* node);
    void addChild(IntermediateNode* node);
    IntermediateNode * getParent();
    IntermediateNode * getChild(uint32_t index);
    uint32_t getNumberChildren();

    IntermediateNode * operator[](uint32_t index);
    ~IntermediateNode();

private:
    IntermediateNode *firstChild = nullptr;
    IntermediateNode *nextSibling = nullptr;
    IntermediateNode *previous = nullptr;
    Token token = Token();
    bool hasParent = false;

    IntermediateNode * getSibling(uint32_t index);
    uint32_t getNumberYoungerSiblings();

    void destroy();
};

#endif // INTERMEDIATENODE_HPP
