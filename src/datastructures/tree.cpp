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
    enum Direction {
        N, 
        L, 
        R,
    };

    template<class T>
    struct Tree {
        NodeType type;
        T value;
        unsigned int count;
        Blocked blocked = None;
        Direction direction = N;
        int left_count = -1;
        int right_count = -1;
        Tree<T>* left;
        Tree<T>* right;
    };

    template<class T>
    struct SearchNode {
        Tree<T>* node;
        Direction direction;
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
            leaf->left_count = leaf->right_count = -1;
            leaf->blocked = None;
            leaf->direction = N;
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
            node->left_count = node->right_count = -1;
            node->blocked = None;
            node->direction = N;
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

    template <class T>
    static std::vector<SearchNode<T>> query_tree(Tree<T>* tree, T query_value, std::function<bool(T, T)> pred) {
        SearchNode<T> root;
        root.node = tree;

        std::vector<SearchNode<T>> search_path = { root };
        while (tree->type != Leaf) {
            if (pred(query_value, tree->value)) {
                (*(search_path.end() - 1)).direction = L;
                tree = tree->left;
            }
            else {
                (*(search_path.end() - 1)).direction = R;
                tree = tree->right;
            }
            SearchNode<T> new_node;
            new_node.node = tree;
            search_path.push_back(new_node);
        }
        (*(search_path.end() - 1)).direction = pred(query_value, tree->value) ? L : R;
        return search_path;
    }

    template <class T>
    static void set_tree_counts(std::vector<SearchNode<T>> search_path) {
        // Set all nodes to be split; used in later step
        for (auto i = search_path.begin(); i < search_path.end(); i++) {
            i->node->blocked = Split;
            i->node->direction = i->direction;
        }

        // TODO: combine this, less pointer access
        // First, create counts for the left split
        for (int i = (int)search_path.size() - 1; i >= 0; i--) {
            if (search_path[i].node->type == Leaf) {
                search_path[i].node->left_count = search_path[i].direction == L ? 0 : 1;
            }
            else {
                auto left_contribution = search_path[i].node->left->left_count == -1 ? search_path[i].node->left->count : search_path[i].node->left->left_count;
                auto right_contribution = search_path[i].direction == R ? search_path[i].node->right->left_count : 0;
                search_path[i].node->left_count = left_contribution + right_contribution;
            }
        }

        // Then, basically do the same thing for the right split but in reverse
        for (int i = (int)search_path.size() - 1; i >= 0; i--) {
            if (search_path[i].node->type == Leaf) {
                search_path[i].node->right_count = search_path[i].direction == R ? 0 : 1;
            }
            else {
                auto right_contribution = search_path[i].node->right->right_count == -1 ? search_path[i].node->right->count : search_path[i].node->right->right_count;
                auto left_contribution = search_path[i].direction == L ? search_path[i].node->left->right_count : 0;
                search_path[i].node->right_count = left_contribution + right_contribution;
            }
        }
    }

    template <class T>
    static void reset_tree_counts(std::vector<SearchNode<T>> search_path) {
        for (auto i = search_path.begin(); i < search_path.end(); i++) {
            i->node->left_count = i->node->right_count = -1;
            i->node->blocked = None;
            i->node->direction = N;
        }
    }

    template <class T>
    // Takes a tree which is already split by a searchpath for q
    static Tree<T>* find_k_nearest(Tree<T>* tree, int k) {
#ifdef DEBUG_PRINT
        if (tree->left_count == -1 || tree->right_count == -1)
            std::cout << "attempting to search in unsplit tree!!" << std::endl;
#endif // DEBUG_PRINT
        auto left_root = tree, right_root = tree;
        // left is blue, right is red.
        // erwin als je hiernaar kijkt: het spijt me
        while (left_root->type != Leaf && right_root->type != Leaf) {
            auto left_contribution = left_root->blocked == Split ?
                (left_root->direction == R ? left_root->right->left_count : 0)
                : (left_root->right->left_count == -1 ? left_root->right->count : left_root->right->left_count);
            auto right_contribution = right_root->blocked == Split ?
                (right_root->direction == L ? right_root->left->right_count : 0)
                : (right_root->blocked == Left ? 0 
                : (right_root->left->right_count == -1 ? right_root->left->count : right_root->left->right_count));
            int l = left_contribution + right_contribution + k;
            if (l <= k) {
                left_root = left_root->left;
                right_root->blocked = Left;
                k -= l;
            }
            else {
                right_root = right_root->left;
            }
        }

        return left_root->type == Leaf ? left_root : right_root;
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