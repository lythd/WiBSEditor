/* binarytreehelper.hpp
PURPOSE:
- Provides static helper functions for dealing with binary trees
- Binary trees can be stuctured as a vector, where you can get from an index to the parent index by index//2, and to the children by index*2+1 and index*2+2
*/
#ifndef BINARYTREEHELPER_HPP
#define BINARYTREEHELPER_HPP

#include <cmath>

class BinaryTreeHelper {
public:
    // Function to calculate the depth (row) of a node in a binary tree
    static int getDepth(int index) {
        return std::floor(std::log2(index + 1));
    }

    // Function to calculate the column for a node based on its depth and index
    static int getColumn(int index, int depth) {
        // Find the relative position within the level
        return index - (std::pow(2, depth) - 1);
    }

    // Function to calculate the maximum number of columns in a row for a given depth
    static int getMaxColumnsForDepth(int depth) {
        return std::pow(2, depth);
    }
};

#endif // BINARYTREEHELPER_HPP
