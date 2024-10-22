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
    void addSibling(IntermediateNode* node);
    void addChild(IntermediateNode* node);
    IntermediateNode * getParent();
    IntermediateNode * getChild(uint index);
    uint getNumberChildren();

    IntermediateNode * operator[](uint index);
    ~IntermediateNode();

private:
    IntermediateNode *firstChild = nullptr;
    IntermediateNode *nextSibling = nullptr;
    IntermediateNode *previous = nullptr;
    Token token = Token();
    bool hasParent = false;

    IntermediateNode * getSibling(uint index);
    uint getNumberYoungerSiblings();

    void destroy();
};

#endif // INTERMEDIATENODE_HPP
