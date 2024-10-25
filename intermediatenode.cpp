/* intermediatenode.cpp
PURPOSE:
- Takes in tokens and produces an intermediate structure for use in exporting based on phrases
*/
#include "intermediatenode.h"
#include <cstdint>

void IntermediateNode::generateTree(std::vector<std::tuple<std::string, uint32_t, uint32_t>> tokens) {
    if (token.getType() != Token::TokenType::UNSET) destroy();

    IntermediateNode *lastTopLevel = nullptr; // The last top level node
    IntermediateNode *last = nullptr; // The last childless node so that it is the bottom, the place where we are adding from
    IntermediateNode *lastlast = last; // The previous value in last, likely not a childless/bottom node

    for (std::tuple<std::string, uint32_t, uint32_t> tuple : tokens) {
        std::string value = std::get<0>(tuple);
        uint32_t line = std::get<1>(tuple);
        uint32_t pos = std::get<2>(tuple);
        bool first = true, inLink = false, inHtml = false;
        if (last != nullptr) {
            if (last->token.getType() == Token::TokenType::KEYWORD &&
                    (last->token.getValue() == "open" ||
                    last->token.getValue() == "file"))
                inLink = true;
            else if (last->getParent() != nullptr &&
                    (last->getParent()->token.getType() == Token::TokenType::FILE_LITERAL ||
                    (last->getParent()->token.getType() == Token::TokenType::KEYWORD &&
                    (last->getParent()->token.getValue() == "open" ||
                    last->getParent()->token.getValue() == "file"))))
                inLink = true;
            if (Token::getPhraseLength(last->token) > last->getNumberChildren())
                first = last->getNumberChildren() > 0;
            else if (last->getParent() != nullptr)
                first = Token::getPhraseLength(last->getParent()->token) <= last->getParent()->getNumberChildren();
            if (last->token.getType() == Token::TokenType::KEYWORD &&
                    last->token.getValue() == "create")
                inHtml = true;
        }
        Token cToken = Token(Token::getLiteral(value, inLink), value, line,
                pos, first, inLink, inHtml);

        // We have to go through some special cases before getting to the nice stuff

        // The first token is always special, it just becomes the first token
        if (last == nullptr) {
            lastlast = nullptr;
            last = this;
            lastTopLevel = this;
            token = cToken;
            continue;
        }

        // Assignments are always special
        if (cToken.getType() == Token::TokenType::ASSIGNMENT) {
            // If the last element is not part of a const or argument list automatically assume equality
            if (last->getParent() == nullptr ||
                    (last->getParent()->token.getType() != Token::TokenType::CONST &&
                    last->getParent()->token.getType() != Token::TokenType::ARGUMENT_LIST))
                cToken = Token(Token::TokenType::BINARY_OPERATOR, "=", // This does mean there is both a "=" and "==" binary operator that function the same, but if I were to make this == you wouldn't be able to explicitly type "==" for the binary equality
                        cToken.getLine(), cToken.getPos());
            // Double equals will always be equality
            if (last->getParent() != nullptr && last->getParent()->getNumberChildren() == 1 &&
                    (last->getParent()->token.getType() == Token::TokenType::BINARY_OPERATOR ||
                    last->getParent()->token.getType() == Token::TokenType::ASSIGNMENT) &&
                    last->getParent()->token.getValue() == "=") {
                last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "==",
                        last->getParent()->token.getLine(), last->getParent()->token.getPos());
                continue; // Just need to adjust and move on since it's not a new token
            }
            // Can merge with '>' and '<'
            if (last->getParent() != nullptr && last->getParent()->getNumberChildren() == 1 &&
                    last->getParent()->token.getType() == Token::TokenType::BINARY_OPERATOR) {
                if (last->getParent()->token.getValue() == "<")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "≤",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                if (last->getParent()->token.getValue() == ">")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "≥",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                continue; // Just need to adjust and move on since it's not a new token
            }
            // Can also merge with '!' and '~'
            if (last->token.getType() == Token::TokenType::UNARY_OPERATOR) {
                if (last->token.getValue() == "!")
                    last->token = Token(Token::TokenType::BINARY_OPERATOR, "≠",
                            last->token.getLine(), last->token.getPos());
                if (last->token.getValue() == "~")
                    last->token = Token(Token::TokenType::BINARY_OPERATOR, "≈",
                            last->token.getLine(), last->token.getPos());
                // Since as a unary operator it would have started its own phrase and left like the LHS of this expression we need to merge
                if (lastlast != nullptr) {
                    if (lastlast->previous != nullptr) {
                        if (lastlast->hasParent) lastlast->previous->firstChild = last;
                        else lastlast->previous->nextSibling = last;
                    } if (lastlast->nextSibling != nullptr) lastlast->nextSibling->previous = last;
                    last->hasParent = lastlast->hasParent;
                    last->previous = lastlast->previous;
                    last->firstChild = lastlast;
                    last->nextSibling = lastlast->nextSibling;
                    lastlast->hasParent = true;
                    lastlast->previous = last;
                    lastlast->nextSibling = nullptr;
                    last = lastlast; // These two lines just swap last and lastlast
                    lastlast = last->previous;
                    }
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
            lastlast = newNode;
            continue;
        }

        // Binary Operators are always special
        if(cToken.getType() == Token::TokenType::BINARY_OPERATOR) {
            // '*', '/', '^', '&', and '|', can all double up
            if (last->getParent() != nullptr && last->getParent()->getNumberChildren() == 1 &&
                    last->getParent()->token.getType() == Token::TokenType::BINARY_OPERATOR && 
                    last->getParent()->token.getValue() == cToken.getValue()) {
                if(cToken.getValue() == "*")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "**",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                else if(cToken.getValue() == "/")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "//",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                else if(cToken.getValue() == "^")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "xor",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                else if(cToken.getValue() == "&")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "and",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                else if(cToken.getValue() == "|")
                    last->getParent()->token = Token(Token::TokenType::BINARY_OPERATOR, "or",
                            last->getParent()->token.getLine(), last->getParent()->token.getPos());
                continue; // Just need to adjust and move on since it's not a new token
            }

            // '/' merges with file literals
            if (cToken.getValue() == "/" &&
                    last->token.getType() == Token::TokenType::FILE_LITERAL) {
                last->token.setValue(last->token.getValue() + "/");
                continue; // Merging so not creating a new token
            }
            
            // Otherwise gobble up, replace last node and then have it as a child
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
            lastlast = newNode;
            continue;
            
            // TODO: I suppose it is right above where I would deal with reordering, like this one should always be put below if the last one was an operation unless this one was higher priority (strictly higher since it came after)
        }

        // Unary Operators, only the unary '/' (only meant to be used for starting a file literal)
                // and the unary '(' since it might be an argument list '(' are special
        if (cToken.getType() == Token::TokenType::UNARY_OPERATOR)
            // A unary '/' just makes a blank file literal, allowing you to make one at any point if you so wanted, though again it is just a string its not typed
                    // just has some extra features in that you can be warned if it's not found, and it can be placed in subdirectories and still be found
            if (cToken.getValue() == "/")
                cToken = Token(Token::TokenType::FILE_LITERAL, "", cToken.getLine(), cToken.getPos());
            else if (cToken.getValue() == "(") {
                // Make argument expression if possible, since it only replaces a value expression or an htmlpart it will always be acceptable so no need to check
                if (last->token.getType() == Token::TokenType::HTMLPART || 
                        (Token::isValueExpression(last->token) &&
                        last->isComplete())) {
                    // Make a binary '(' and swap it with last
                    IntermediateNode *newNode = new IntermediateNode();
                    newNode->token = Token(Token::TokenType::BINARY_OPERATOR, cToken.getValue(), cToken.getLine(), cToken.getPos());
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
                    lastlast = newNode;
                    // Make cToken an argument list and carry on
                    cToken = Token(Token::TokenType::ARGUMENT_LIST, cToken.getValue(), cToken.getLine(), cToken.getPos());
                }
            }

        // File literals, only thing special is to merge with previous ones if there are any
        else if (cToken.getType() == Token::TokenType::FILE_LITERAL) {
            if (last->token.getType() == Token::TokenType::FILE_LITERAL) {
                if ((last->token.getValue().size() == 0 || last->token.getValue().back() == '/') ||
                        (cToken.getValue().size() == 0 || cToken.getValue().front() == '/'))
                    last->token.setValue(last->token.getValue() + cToken.getValue());
                    // If neither literal has a '/' and neither is blank then add a slash when conjoining
                else last->token.setValue(last->token.getValue() + "/" + cToken.getValue());
                continue; // Merging so not creating a new token
            }
        }

        // Filler, closing brackets are special, remember we don't know which type of ')' we have (either unary or argument list, the binary one doesn't need closing cause its paired with argument list)
        else if (cToken.getType() == Token::TokenType::FILE_LITERAL) {
            std::string match = "";
            if (cToken.getValue() == ")") match = "(";
            else if (cToken.getValue() == "]") match = "[";
            if (match != "") {
                IntermediateNode *lastp = last;
                // Do not match with a binary '(', only matching with unary '(', arg list '(', or list literal '['
                // Binary '(' will never need matching
                // To avoid string literals and others we explicitly type unary, arg list, or list literal.
                while (!(lastp->token.getValue() == match && (lastp->token.getType() == Token::TokenType::UNARY_OPERATOR || 
                        lastp->token.getType() == Token::TokenType::ARGUMENT_LIST || lastp->token.getType() == Token::TokenType::LIST_LITERAL))) {
                    lastp = lastp->getParent();
                    if (lastp == nullptr) break; // Will go on to add it as a literal and cause a syntax error for a mismatched bracket.
                }
                // If we are here we have thus found the matching bracket and can close it and then continue
                lastp->token.setValue(match + cToken.getValue());
                // If it is not a unary operator then is some kind of list and we remove any potential trailing comma child
                if (lastp->token.getType() != Token::TokenType::UNARY_OPERATOR) {
                    if (auto *lc = (*lastp)[-1]; lc != nullptr && lc->token.getValue() == "," && lc->token.getType() == Token::TokenType::FILLER) {
                        lc->disconnect();
                        // Since it has a parent we can safely call disconnect(), a comma literal should never have a child, and we got it by it being the last child, so it shouldn't have any children or siblings anyway, but to be safe calling disconnect to avoid deleting them
                        delete lc;
                    }
                }
                continue;
            }
        }

        // Add as child as last if it needs and can be added, if not keep going to the parent up
        IntermediateNode *lastp = last;
        while (lastp != nullptr) {
            if (!lastp->isComplete() && Token::doesAcceptInPosition(lastp->token, cToken, lastp->getNumberChildren())) {
                // Make and add as a child
                IntermediateNode *node = new IntermediateNode();
                node->token = cToken;
                lastp->addChild(node);
                lastlast = last;
                last = node;
                // Argument lists and regular lists need an initial ',' filler as a first child
                if (cToken.getType() == Token::TokenType::ARGUMENT_LIST || cToken.getType() == Token::TokenType::LIST_LITERAL) {
                    IntermediateNode *node2 = new IntermediateNode();
                    node2->token = Token(Token::TokenType::FILLER, ",", cToken.getLine(), cToken.getPos());
                    last->addChild(node2);
                    lastlast = last;
                    last = node2;
                }
                break;
            }
            lastp = lastp->getParent();
        }

        // If you couldn't find any then make a sibling of the lasttoplevel
        // This is also the only time we update lastTopLevel
        if (lastp == nullptr) {
            IntermediateNode *node = new IntermediateNode();
            node->token = cToken;
            lastTopLevel->addSibling(node);
            lastlast = last;
            last = node;
            lastTopLevel = node;
        }
    }
}

// Goes through the tree and compiles a list of errors so that the editor window can
// squiggle and so that it can be displayed as a list of text in a popup.
std::vector<SyntaxError> getErrors() {

}

bool IntermediateNode::isComplete() {
    uint32_t target = Token::getPhraseLength(token);
    uint32_t children = getNumberChildren();
    IntermediateNode *child = getChild(-1);
    if (target == (uint32_t)-1) {
        if (children == 0) return true; // No comma means it has been removed and you are done, just happens to be empty
        // For argument lists if the last child is a name then incomplete
        if (child->token.getType() == Token::TokenType::NAME &&
                token.getType() == Token::TokenType::ARGUMENT_LIST) return false;
        // If the value is either just a '(' or just a '[' it has not been matched with a closing bracket and so is not complete
        if (token.getValue() == "(" || token.getValue() == "[") return false;
        // Recursiveness
        return child->isComplete();
    }
    if (target > getNumberChildren()) return false;
    // More children would be incorrect too, but it is complete, so that would be a separate validation to check for that
    // Finally make sure the last child if applicable is complete
    if (children == 0) return true;
    return child->isComplete();
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


// Negative indices the size gets added, gives nullptr for anything too negative or too positive that it exceeds
IntermediateNode * IntermediateNode::getChild(int32_t index) {
    if (index < 0) {
        index += getNumberChildren();
        if (index < 0) return nullptr;
    }
    if (index == 0) return firstChild;
    if (firstChild == nullptr) return nullptr;
    return firstChild->getSibling(index-1);
}

// Its only purpose was to complete the getChild implementation
// Gets sibling with relative index
// Returns nullptr for negatives or if the index is too high
IntermediateNode * IntermediateNode::getSibling(int32_t index) {
    if (index < 0) return nullptr;
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

// Just calls getChild(), look there for details
IntermediateNode * IntermediateNode::operator[](int32_t index) {
    return getChild(index);
}

IntermediateNode::~IntermediateNode() {
    destroy();
}

// Dangerous since it can leave stranded bits of the tree
void IntermediateNode::disconnect() {
    // If there is a previous then it won't leave anything stranded it will connect
    if (previous != nullptr) {
        if (hasParent) {
            // First child replaces it
            if (firstChild != nullptr) {
                firstChild->previous = previous;
                previous->firstChild = firstChild;
                // firstChild->hasParent would already be set to true
                // If there is a next sibling it will become a sibling of the child, still stranded though, this will likely break any syntax but if this is happening
                        // syntax is already broken, and we are preventing memory leaks.
                if (nextSibling != nullptr) firstChild->addSibling(nextSibling);
            }
            // Next sibling replaces it
            else if (nextSibling != nullptr) {
                nextSibling->previous = previous;
                previous->firstChild = nextSibling;
                nextSibling->hasParent = true;
            }
            // Otherwise simple disconnect
            else previous->firstChild = nullptr;
        } else {
            // First child replaces it
            if (firstChild != nullptr) {
                firstChild->previous = previous;
                previous->nextSibling = firstChild;
                firstChild->hasParent = false;
                // If there is a next sibling it will become a sibling of the child, still stranded though, this will likely break any syntax but if this is happening
                        // syntax is already broken, and we are preventing memory leaks.
                if (nextSibling != nullptr) firstChild->addSibling(nextSibling);
            }
            // Next sibling replaces it
            else if (nextSibling != nullptr) {
                nextSibling->previous = previous;
                previous->nextSibling = nextSibling;
                // nextSibling->hasParent would already be set to false
            }
            // Otherwise simple disconnect
            else previous->nextSibling = nullptr;
        }
    }

    // No parent means any children or siblings become stranded, bad!
    else if (firstChild != nullptr) {
        // If there is a next sibling it will become a sibling of the child, still stranded though, this will likely break any syntax but if this is happening
                // syntax is already broken, and we are preventing memory leaks.
        if (nextSibling != nullptr) firstChild->addSibling(nextSibling);
        firstChild->hasParent = false;
        firstChild->previous = nullptr;
    }

    // Just the next sibling, which will become stranded, bad!
    else if (nextSibling != nullptr) nextSibling->previous = nullptr;

    token = Token();
    previous = nullptr;
    firstChild = nullptr;
    nextSibling = nullptr;
    hasParent = false;
}

// Deletes younger siblings and children too to prevent fragmentation and also because you often want to do that
void IntermediateNode::destroy() {
    token = Token();
    if (previous != nullptr) {
        if (hasParent) previous->firstChild = nullptr;
        else previous->nextSibling = nullptr;
        previous = nullptr;
    }
    hasParent = false;
    if (firstChild != nullptr) firstChild->destroy();
    if (nextSibling != nullptr) nextSibling->destroy();
    delete firstChild;
    delete nextSibling;
    firstChild = nullptr;
    nextSibling = nullptr;
}