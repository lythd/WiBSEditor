/* intermediatenode.cpp
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/

#include "intermediatenode.h"

void IntermediateNode::generateTree(std::vector<std::string> tokens) {
    if (value != "") destroy();

    IntermediateNode *lastTopLevel = nullptr;
    IntermediateNode *last = nullptr;

    for(std::string token : tokens) {
        
    }
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

IntermediateNode * IntermediateNode::getChild(uint index) {
    if (index == 0) return firstChild;
    if (firstChild == nullptr) return nullptr;
    return firstChild->getSibling(index-1);
}

IntermediateNode * IntermediateNode::getSibling(uint index) {
    if (index == 0) return nextSibling;
    if (nextSibling == nullptr) return nullptr;
    return nextSibling->getSibling(index-1);
}

uint IntermediateNode::getNumberChildren() {
    if (firstChild == nullptr) return 0;
    return 1 + firstChild->getNumberYoungerSiblings();
}

uint IntermediateNode::getNumberYoungerSiblings() {
    if (nextSibling == nullptr) return 0;
    return 1 + nextSibling->getNumberYoungerSiblings();
}

IntermediateNode * IntermediateNode::operator[](uint index) {
    return getChild(index);
}

IntermediateNode::~IntermediateNode() {
    destroy();
}

void IntermediateNode::destroy() {
    value = "";
    hasParent = false;
    previous = nullptr;
    if (firstChild != nullptr) firstChild->destroy();
    if (nextSibling != nullptr) nextSibling->destroy();
    delete firstChild;
    delete nextSibling;
}