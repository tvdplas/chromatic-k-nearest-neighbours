#pragma once
#include <iostream>
#include <vector>
#include <functional>

// im just testing stuff out at this point, but macros are fun!
#define casted_malloc(type) (type)malloc(sizeof(type))
#define for_each(pointer_name, items) for(auto pointer_name = items.begin(); pointer_name < items.end(); pointer_name++)

namespace DS {
	enum Side {
		None = 0,
		Left = 1, 
		Right = 2,
	};

	enum SortedSide {
		LessThan,
		GreaterThan,
	};
	
	static Side flip(Side side) {
		return side == Left ? Right : Left;
	}

	enum Color {
		Red,
		Blue,
	};

	static Color flip(Color color) {
		return color == Red ? Blue : Red;
	}

	// leafs are simply nullptrs
	template <class T> struct Tree {
		T value;
		int unsplit_count = 1;
		int left_count = -1;
		int right_count = -1;
		Side direction = None;
		Tree<T>* left = nullptr;
		Tree<T>* right = nullptr;
	};

	template <class T> struct SearchedNode {
		Tree<T>* node;
		Side direction;
	};
	
	// Generates new tree. Input must already be a sorted list, because I am lazy
	template <class T> Tree<T>* generate_tree(std::vector<T> items) { return generate_tree(items, 0, (int)items.size() - 1); }
	template <class T> Tree<T>* generate_tree(std::vector<T> items, int begin, int end) {
		Tree<T>* node = casted_malloc(Tree<T>*);
		node = new Tree<T>;
		if (begin > end) {
			return nullptr; // leaf
		}
		else if (begin == end) {
			node->value = items[begin];
		}
		else {
			int middle = (begin + end) / 2;
			auto right = generate_tree(items, middle + 1, end);
			node->value = items[middle];
			node->left = generate_tree(items, begin, middle - 1);
			node->right = generate_tree(items, middle + 1, end);
			node->unsplit_count = 1 + 
				(node->left == nullptr ? 0 : node->left->unsplit_count)
			  + (node->right == nullptr ? 0 : node->right->unsplit_count);
		}

		return node;
	}

	// Gets a search path for a given query point, along with the direction that was taken during the search
	template <class T> std::vector<SearchedNode<T>> get_search_path(Tree<T>* tree, T query_point, std::function<bool(T, T)> leq) {
		SearchedNode<T> root = { tree };
		std::vector<SearchedNode<T>> search_path = { root };
		while (tree) {
			auto direction = leq(query_point, tree->value) ? Left : Right;
			(*(search_path.end() - 1)).direction = direction;

			tree = direction == Left ? tree->left : tree->right;
			SearchedNode<T> next = { tree };
			search_path.push_back(next);
		}
		search_path.pop_back();
		return search_path;
	}

	// set count for certain split
	template <class T> void set_side_count(Tree<T>* tree, int count, Side split_side) {
		if (split_side == Left) {
			tree->left_count = count;
		}
		else {
			tree->right_count = count;
		}
	}
	// get the count of a tree node given the fact that it is in a split side
	template <class T> int get_side_count(Tree<T>* tree, Side split_side) {
		if (!tree) return 0; // leaf

		if (split_side == Left) {
			tree->left_count == -1 ? tree->unsplit_count : tree->left_count;
		}
		else {
			tree->right_count == -1 ? tree->unsplit_count : tree->right_count;
		}
	}
	// get the count of a tree node sort side given the fact that it is in a split side
	template <class T> int get_side_count(Tree<T>* tree, SortedSide sorted_side, Side split_side) {
		auto child = get_side(tree, sorted_side, split_side);
		return get_side_count(child, split_side);
	}

	// set the left/right side of tree
	template <class T> void set_side(Tree<T>* parent, Tree<T>* child, Side side) {
		if (side == Left) {
			parent->left= child;
		}
		else {
			parent->right = child;
		}
	}
	// get the left/right side of tree
	template <class T> Tree<T>* get_side(Tree<T>* tree, Side side) {
		return side == Left ? tree->left : tree->right;
	}

	// get the LE/GE side of tree depending on split side
	template <class T> Tree<T>* get_side(Tree<T>* tree, SortedSide sorted_side, Side split_side) {
		return ((split_side == Left) == (sorted_side == LessThan)) ? tree->right : tree->left;
	}


	// Splits a tree along a given search path
	// TODO: return a list of modified node pointers, along with copies of their original contents.
	// This way, we can restore in O(lg n) time.
	template <class T> std::vector<Tree<T>*> split_tree(std::vector<SearchedNode<T>> search_path) {
		// First, update tree such that directions are stored.
		// Is used in order to determine bounds during count calculations
		for_each(sn, search_path) {
			 sn->node->direction = sn->direction;
		}

		// Now, we begin hell :tm: 
		// We need to construct two semi-balanced trees from the mess that we created, while also updating counts correctly
		// We start with the former, by always "glueing" a search path node to the previous entry in the tree
		// see https://pasteboard.co/qQWyoic20926.jpg for a rough sketch

		// the two new roots
		Tree<T>* left_root = nullptr, * right_root = nullptr;
		// the most recent in the sequence; used to append to.
		Tree<T>* left_latest = nullptr, * right_latest = nullptr;
		for (int i = 0; i < search_path.size(); i++) {
			Tree<T>* node = search_path[i].node;
				
			// If search direction was right, the node itself must be part of left tree and vice versa
			auto addition_target = node->direction == Left ? &right_latest : &left_latest;

			// No root was yet set for this side of the tree; we will set it now.
			// As there was no root yet, no children need to be updated yet. 
			if (*addition_target == nullptr) {
				if (node->direction == Right) left_root = left_latest = node;
				else right_root = right_latest = node;
			}
			else {
				// Otherwise, the latest node needs to be connected to our current node. 
				// Afterwards, it is updated for possible future addition.
				set_side(*addition_target, node, node->direction); // node direction and addition direction are always the same
				*addition_target = node;
			}
			
			// edge case; if no more children our found for this node, dont link to other side of tree
			// due to existing children
			set_side(*addition_target, (Tree<T>*)nullptr, node->direction);
		}

		// Now that the trees are split, we can update the counts. Only the counts along the splitting edge are affected,
		// and we can propegate the changes from the bottom up. 
		for (int i = search_path.size() - 1; i >= 0; i--) {
			auto node = search_path[i].node;
			 // First, get the side that might have been affected earlier
			auto affected_child = get_side(node, node->direction);
			auto non_affected_child = get_side(node, flip(node->direction));

			// Then, get its count, and add it to the unaffected side's count to get ours (+1 for own contribution)
			// get_side_count only used for convenience for non_affected, it should always return normal count
			int our_count = get_side_count(affected_child, flip(node->direction)) + get_side_count(non_affected_child, flip(node->direction)) + 1;
			set_side_count(node, our_count, flip(node->direction));
		}

		return { left_root, right_root };
	}

	// Get the k nearest neighbour in a single tree, measured from the smallest side of the tree
	// Todo: test individually.
	template <class T> T get_k_nearest_single(Tree<T>* tree, int k, Side split_side) {
		if (k > get_side_count(tree, split_side))
			std::cout << "ERROR: attempting to get k=" << k << " neighbour in tree of size " << get_side_count(tree, split_side);

		while (tree->left != nullptr || tree->right != nullptr) {
			int closerCount = get_side_count(tree, LessThan, split_side);

			if (closerCount + 1 == k) {
				// root is kth neighbour!
				return tree->value;
			}
			else if (closerCount + 1 < k) {
				// root and LE side aren't it, therefore we continue on greater side
				k -= closerCount + 1;
				tree = get_side(tree, GreaterThan, split_side);
			}
			else {
				// root and GE side aren't it, therefore continue on smaller side
				tree = get_side(tree, LessThan, split_side);
			}
		}
		return tree->value;
	}

	template <class T> T get_k_nearest(Tree<T>* left, Tree<T>* right, int k, T q, std::function<double(T, T)> distance) {
		Tree<T>* red = right, * blue = left;
		Side blue_side = Left;
		
		while (red != nullptr && blue != nullptr) {
			if (distance(q, blue->value) > distance(q, red->value)) {
				auto temp = red;
				red = blue;
				blue = temp;
				blue_side = flip(blue_side);
			}

			int blue_le_count = get_side_count(blue, LessThan, blue_side);
			int red_le_count = get_side_count(red, LessThan, flip(blue_side));
			int l = blue_le_count + red_le_count + 1; // +1 is back >:)
			std::cout << "R/B status after correction: " << std::endl;
			std::cout << "Red " << (blue_side == Left ? "(right): " : "(left): ") << "value: " << red->value << " count: " << get_side_count(red, flip(blue_side)) << std::endl;
			std::cout << "Blue " << (blue_side == Right ? "(right): " : "(left): ") << "value: " << blue->value << " count: " << get_side_count(blue, blue_side) << std::endl;
			std::cout << "red_leq: " << red_le_count << std::endl;
			std::cout << "blue_leq: " << blue_le_count << std::endl;
			std::cout << "l: " << l << std::endl;
			std::cout << "k: " << k << std::endl;
			if (l == k) {
				// then our target must be in b or B< or R<
				red = get_side(red, LessThan, flip(blue_side));
				auto blue_ge_side = get_side(blue, GreaterThan, blue_side);
				blue_ge_side = nullptr;
			}
			else if (l < k) {
				blue = get_side(blue, GreaterThan, blue_side);
				k -= blue_le_count + 1;
			}
			else {
				red = get_side(red, LessThan, flip(blue_side));
			}
		}

		// Now get kth in remaining tree
		auto rt = red == nullptr ? blue : red;
		auto rt_side = red == nullptr ? blue_side : flip(blue_side);
		return get_k_nearest_single(rt, k, rt_side);
	}
}