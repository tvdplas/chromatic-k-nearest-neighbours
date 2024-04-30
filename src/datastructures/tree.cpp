#pragma once
#include <iostream>
#include <vector>
#include <functional>

namespace DS {
    enum NodeType {
        Leaf,
        Internal
    };
    enum Blocked {
        None,
        Left,
        Split,
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
    static Tree<T>* generate_tree(std::vector<T> entries, std::function<T(T,T)> between) {
        if (entries.size() == 0) return nullptr;

        return DS::generate_tree(entries, between, 0, (int)entries.size() - 1);
    }

    template<class T>
    // Takes an ordered list of items as input
    static Tree<T>* generate_tree(std::vector<T> entries, std::function<T(T, T)> between, int begin, int end) {
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
            node->left = generate_tree(entries, between, begin, middle - 1);
            node->right = generate_tree(entries, between, middle, end);
            node->value = between(node->left->value, node->right->value);
            node->count = node->left->count + node->right->count;
            return node;
        }
    }

    template<class T> 
    static void print_tree(Tree<T>* tree, int depth = 0) {
        for (int i = 0; i < depth; i++) std::cout << "  ";
        std::cout << ((tree->type == Leaf) ? "leaf: " : "") << tree->value << std::endl;
        if (tree->type == Internal) {
            print_tree(tree->left, depth + 1);
            print_tree(tree->right, depth + 1);
        }
    }

    template<class T>
    // Returns the first node in the tree which is larger than the 
    // provided query value, determined by the predicate.
    // Also outputs the entire search path 
    static Tree<T>* query_tree(Tree<T>* tree, T query_value, std::function<bool(T, T)> pred, std::vector<Tree<T>*>* search_path) {
        search_path->push_back(tree);
        if (tree->type == Leaf) return tree;
        auto target = pred(query_value, tree->value) ? tree->left : tree->right;
        return query_tree(target, query_value, pred, search_path);
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