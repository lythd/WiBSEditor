/* intermediatenode.h
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/
#ifndef INTERMEDIATENODE_HPP
#define INTERMEDIATENODE_HPP

#include "defines.h"
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
    // Negative indices the size gets added, gives nullptr for anything too negative or too positive that it exceeds
    IntermediateNode * getChild(int32_t index);
    uint32_t getNumberChildren();
    uint32_t getNumberTotal();
    #ifdef DEBUG
    void getAsVector(std::vector<std::string> &vec);
    #endif

    // Just calls getChild(), look there for details
    IntermediateNode * operator[](int32_t index);
    ~IntermediateNode();

private:
    IntermediateNode *firstChild = nullptr;
    IntermediateNode *nextSibling = nullptr;
    IntermediateNode *previous = nullptr;
    Token token = Token();
    // This is whether previous is a parent (as opposed to an older sibling or nullptr), not whether it has a parent at all,
            // that can be checked by comparing getParent() to nullptr
    bool hasParent = false;

    // Its only purpose was to complete the getChild implementation
    // Gets sibling with relative index
    // Returns nullptr for negatives or if the index is too high
    IntermediateNode * getSibling(int32_t index);
    uint32_t getNumberYoungerSiblings();

    // Dangerous since it can leave stranded bits of the tree
    void disconnect();
    // Deletes younger siblings and children too to prevent fragmentation and also because you often want to do that
    void destroy();
};

#endif // INTERMEDIATENODE_HPP
