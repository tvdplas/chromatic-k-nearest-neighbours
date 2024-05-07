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
        Smaller,
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
            node->value = get_max(node->left);
            node->count = node->left->count + node->right->count;
            node->left_count = node->right_count = -1;
            node->blocked = None;
            node->direction = N;
            return node;
        }
    }

    template<class T>
    static T get_max(Tree<T>* tree) {
        while (tree->type != Leaf)
            tree = tree->right;
        return tree->value;
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

        std::cout << "Search path: ";
        for (auto i = search_path.begin(); i < search_path.end(); i++)
            std::cout << (*i).node->value << std::endl;
        std::cout << std::endl;
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
    static T find_k_nearest(Tree<T>* tree, int k, T q, std::function<double(T, T)> distance_from) {
        auto blue = tree, red = tree;
        bool blueIsLeft = true;

        // erwin als je hiernaar kijkt: het spijt me
        while (blue->type != Leaf && red->type != Leaf) {
            // First, determine which tree will be which.
            if (distance_from(q, blue->value) > distance_from(q, red->value)) {
                // switch trees
                auto temp = blue;
                blue = red;
                red = temp;
                blueIsLeft = !blueIsLeft;
            }

            std::cout << "Red (" << (blueIsLeft ? "right): " : "left): ") <<  red->value << std::endl;
            std::cout << "Blue (" << (!blueIsLeft ? "right): " : "left): ") << blue->value << std::endl;

            // Now, we need to determine the contribution of the smaller than side of the red and blue subtree
            int red_contribution, blue_contribution;
            if (blueIsLeft) {
                // Red contribution
                // Red is the right subtree; therefore, the left side of each node is < and the right side is >=
                if (red->blocked == Smaller || red->blocked == Split && red->direction == R) {
                    // Either red is blocked from accesing its smaller side (due to earlier l <= k operation), or the split on q
                    // prevents the tree from accessing the left part of the node. Either way, no count. 
                    red_contribution = 0;
                }
                else if (red->left->right_count >= 0) {
                    // We are not blocked from going left, however we are on one of the seperator nodes. We must now use 
                    // the adjusted count
                    red_contribution = red->left->right_count;
                }
                else {
                    // We are not blocked, and we are in a normal internal node. We can safely use the original count. 
                    red_contribution = red->left->count;
                }

                // Blue contribution
                // Blue is the left subtree here, therefore right side is < and left side is >=
                if (blue->blocked == Smaller || blue->blocked == Split && blue->direction == L) {
                    // Either blue is blocked on the smaller side by unknown operation(due to earlier l <= k operation), or the split on q
                    // prevents the tree from accessing the right part of the node. Either way, no count. 
                    blue_contribution = 0;
                }
                else if (blue->right->left_count >= 0) {
                    // We are not blocked from going right, however we are on one of the seperator nodes. We must now use 
                    // the adjusted count
                    blue_contribution = blue->right->left_count;
                }
                else {
                    // We are not blocked, and we are in a normal internal node. We can safely use the original count. 
                    blue_contribution = blue->right->count;
                }
            }
            else {
                // Same as above, however now orientation of trees is swapped. TODO (that probably wont be done): refactor 
                // Red contribution
                // Red is the left subtree; therefore, the left side of each node is >= and the right side is <
                if (red->blocked == Smaller || red->blocked == Split && red->direction == L) {
                    // Either red is blocked from accesing its smaller side (due to earlier l <= k operation), or the split on q
                    // prevents the tree from accessing the right part of the node. Either way, no count. 
                    red_contribution = 0;
                }
                else if (red->right->left_count >= 0) {
                    // We are not blocked from going right, however we are on one of the seperator nodes. We must now use 
                    // the adjusted count
                    red_contribution = red->right->left_count;
                }
                else {
                    // We are not blocked, and we are in a normal internal node. We can safely use the original count. 
                    red_contribution = red->right->count;
                }

                // Blue contribution
                // Blue is the right subtree here, therefore left side is < and right side is >=
                if (blue->blocked == Smaller || blue->blocked == Split && blue->direction == R) {
                    // Either blue is blocked on the smaller side by unknown operation(due to earlier l <= k operation), or the split on q
                    // prevents the tree from accessing the left part of the node. Either way, no count. 
                    blue_contribution = 0;
                }
                else if (blue->left->right_count >= 0) {
                    // We are not blocked from going left, however we are on one of the seperator nodes. We must now use 
                    // the adjusted count
                    blue_contribution = blue->left->right_count;
                }
                else {
                    // We are not blocked, and we are in a normal internal node. We can safely use the original count. 
                    blue_contribution = blue->left->count;
                }
            }

            // Now that that mess is done, we can determine l and update the trees 
            // No +1 due to r not containing an actual node
            int l = red_contribution + blue_contribution;
            std::cout << "l: " << l << std::endl;
            std::cout << "k: " << k << std::endl;
            std::cout << std::endl;

            if (l <= k) {
                blue = blueIsLeft ? blue->left : blue->right;
                // red doesnt change due to fact that we have no info about R<
                // however, this means that we can only subtract the blue contribution
                // from our target
                k -= blue_contribution;
            }
            else {
                // Depending on whether a greater than node is available, go there
                // Otherwise, fall back to existing node;
                auto target = blueIsLeft ?
                    // we want to go left, but if not available go right
                    (red->blocked == Split && red->direction == R) ? red->right : red->left
                    // we want to go right, but if not available go left
                    : (red->blocked == Split && red->direction == L) ? red->left : red->right;
                red = target;
            }
        }

        return blue->type == Leaf ? blue->value : red->value;
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