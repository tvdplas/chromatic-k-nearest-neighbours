#pragma once
#include <iostream>
#include "generic.cpp"

namespace DS {
    template<class T>
    struct Tree_Node
    {
        T value;
        int children_count;
        Tree_Node<T>* left;
        Tree_Node<T>* right;
    };


    template<size_t SIZE, class T>
    static Tree_Node<T>* generate_tree(T entries[SIZE]) {
        return DS::generate_tree(entries, 0, (int)(SIZE) - 1);
    }

    template<class T>
    // Takes an ordered list of items as input
    static Tree_Node<T>* generate_tree(T entries[], int begin, int end) {
        if (end < begin) {
            return nullptr;
        }
        else if (begin == end) {
            // return leaf-ish
            Tree_Node<T>* node = (Tree_Node<T> *)malloc(sizeof(Tree_Node<T>));
            node->value = entries[begin];
            node->children_count = 1;
            node->left = node->right = nullptr;
            return node;
        }
        else {
            // Create split halfway, recurse on two halves
            int middle = (begin + end) / 2;
            Tree_Node<T>* root = (Tree_Node<T> *)malloc(sizeof(Tree_Node<T>));
            root->value = entries[middle];

            Tree_Node<T>* left = generate_tree(entries, begin, middle - 1);
            Tree_Node<T>* right = generate_tree(entries, middle + 1, end);
            root->children_count = 1;
            if (left) root->children_count += left->children_count;
            if (right) root->children_count += right->children_count;
            root->left = left;
            root->right = right;
            return root;
        }
    }

    template<class T>
    // Free memory allocated for tree
    static void cleanup_tree(Tree_Node<T>* node) {
        if (node->left) cleanup_tree(node->left);
        if (node->right) cleanup_tree(node->right);
        free(node);
    }
}