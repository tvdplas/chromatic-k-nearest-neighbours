#pragma once
#include <iostream>
#include <vector>

namespace DS {
    enum NodeType {
        Leaf,
        Internal
    };
    enum Blocked {
        None,
        Left,
        Right 
    };

    template<class T>
    struct Tree {
        NodeType type;
        T value;
        unsigned int count;
        Blocked blocked;
        Tree<T>* left;
        Tree<T>* right;
    };

    template<class T>
    static Tree<T>* generate_tree(std::vector<T> entries) {
        return DS::generate_tree(entries, 0, (int)entries.size() - 1);
    }

    template<class T>
    // Takes an ordered list of items as input
    static Tree<T>* generate_tree(std::vector<T> entries, int begin, int end) {
        if (begin >= end) {
            Tree<T>* leaf = (Tree<T> *)malloc(sizeof(Tree<T>));
            leaf->type = Leaf;
            leaf->value = entries[begin];
            leaf->count = 1;
            return leaf;
        }
        else {
            int middle = (begin + end) / 2 + 1; // +1 for right biased tree 
            Tree<T>* node = (Tree<T> *)malloc(sizeof(Tree<T>));
            node->type = Internal;
            node->value = entries[middle];
            node->left = generate_tree(entries, begin, middle - 1);
            node->right = generate_tree(entries, middle, end);
            node->count = node->left->count + node->right->count;
            return node;
        }
    }

    template<class T>
    // Free memory allocated for tree
    static void cleanup_tree(Tree<T>* node) {
        if (node->type == Internal) {
            cleanup_tree(node->left);
            cleanup_tree(node->right);
        }
        free(node);
    }
}