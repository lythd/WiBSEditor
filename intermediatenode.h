/* intermediatenode.h
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/
#ifndef INTERMEDIATENODE_HPP
#define INTERMEDIATENODE_HPP

#include <vector>
#include <string>

class IntermediateNode {
public:
    void generateTree(std::vector<std::string> tokens);
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
    std::string value = "";
    bool hasParent = false;

    IntermediateNode * getSibling(uint index);
    uint getNumberYoungerSiblings();

    void destroy();
};

#endif // INTERMEDIATENODE_HPP
