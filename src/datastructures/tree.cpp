#pragma once
#include <iostream>
#include <vector>
#include <functional>

namespace DS {
	enum NodeType {
		Internal = 0,
		Leaf = 1, 
	};

	enum SearchDirection {
		None = 0,
		Left = 1,
		Right = 2,
	};

	enum SplitSide {
		LeftSide = 1,
		RightSide = 2,
	};

	static SplitSide flip(SplitSide side) {
		return (side == LeftSide) ? RightSide : LeftSide;
	}

	enum TreeSide {
		LEQ, 
		GE,
	};

	static TreeSide flip(TreeSide side) {
		return (side == LEQ) ? GE : LEQ;
	}


	template <class T> struct Tree {
		NodeType type;
		T value;
		int unsplit_count;
		int left_split_count = -1;
		int right_split_count = -1;
		Tree* left = nullptr;
		Tree* right = nullptr;
		SearchDirection direction = None;
	};

	template <class T> struct SearchNode {
		Tree<T>* node;
		SearchDirection direction;
	};

	template <class T> Tree<T>* generate_tree(std::vector<T> sorted_items) {
		return generate_tree(sorted_items, 0, (int)sorted_items.size() - 1);
	}

	template <class T> Tree<T>* generate_tree(std::vector<T> sorted_items, int begin, int end) {
		if (begin >= end) {
			Tree<T>* leaf = (Tree<T>*)malloc(sizeof(Tree<T>));
			leaf = new Tree<T>;
			leaf->type = Leaf;
			leaf->value = sorted_items[begin];
			leaf->unsplit_count = 1;
			return leaf;
		}
		else {
			int middle = (begin + end) / 2 + 1;
			Tree<T>* node = (Tree<T> *)malloc(sizeof(Tree<T>));
			node = new Tree<T>;
			node->type = Internal;
			node->left = generate_tree(sorted_items, begin, middle - 1);
			node->right = generate_tree(sorted_items, middle, end);
			node->value = get_max(node->left);
			node->unsplit_count = node->left->unsplit_count + node->right->unsplit_count;
			return node;
		}
	}

	template <class T> bool is_leaf(Tree<T>* tree) {
		return tree->type == Leaf;
	}

	template <class T> T get_max(Tree<T>* tree) {
		while (!is_leaf(tree)) {
			tree = tree->right;
		}
		return tree->value;
	}

	template <class T> std::vector<SearchNode<T>> get_search_path(Tree<T>* tree, T query_point, std::function<bool(T,T)> leq) {
		SearchNode<T> root = { tree };
		std::vector<SearchNode<T>> search_path = { root };
		while (!is_leaf(tree)) {
			auto direction = leq(query_point, tree->value) ? Left : Right;
			(*(search_path.end() - 1)).direction = direction;

			tree = direction == Left ? tree->left : tree->right;
			SearchNode<T> next = { tree };
			search_path.push_back(next);
		}
		(*(search_path.end() - 1)).direction = leq(query_point, tree->value) ? Left : Right;
		return search_path;
	}
	
	template <class T> Tree<T>* get_tree_side(Tree<T>* tree, SplitSide split_side, TreeSide tree_side) {
		if (is_leaf(tree)) {
			std::cout << "ERROR: TRIED TO GET CHILD OF LEAF";
			return nullptr;
		}

		if (tree->direction == Right && split_side == RightSide && tree_side == LEQ) {
			std::cout << "ERROR: CANNOT ENTER BLOCKED PART OF TREE (R, R, LEQ)";
			return nullptr;
		}
		if (tree->direction == Left && split_side == LeftSide && tree_side == LEQ) {
			std::cout << "ERROR: CANNOT ENTER BLOCKED PART OF TREE (L, L, LEQ)";
			return nullptr;
		}
		return (split_side == LeftSide) == (tree_side == LEQ) ? tree->right : tree->left;
	}
	// get the count on a certain side of a tree, given which split we are in
	template <class T> int get_split_count(Tree<T>* tree, SplitSide split_side, TreeSide tree_side) {
		if (is_leaf(tree)) 
			return get_split_count(tree, split_side);

		auto target = get_tree_side(tree, split_side, tree_side);
		if (target == nullptr) return 0;

		auto other = get_tree_side(tree, split_side, flip(tree_side));
		return get_split_count(target, split_side) + (is_leaf(other) ? 1 : 0);
	}
	// get the count of a tree, given which split we are in 
	template <class T> int get_split_count(Tree<T>* tree, SplitSide side) {
		if (side == LeftSide) {
			return tree->left_split_count == -1 ? tree->unsplit_count : tree->left_split_count;
		}
		else {
			return tree->right_split_count == -1 ? tree->unsplit_count : tree->right_split_count;
		}
	}

	template <class T> void split_tree(std::vector<SearchNode<T>> search_path) {
		for (auto i = search_path.begin(); i < search_path.end(); i++) 
			i->node->direction = i->direction;
		
		for (int i = (int)search_path.size() - 1; i >= 0; i--) {
			Tree<T>* node = search_path[i].node;
			if (is_leaf(node)) {
				node->left_split_count = node->direction == Left ? 0 : 1;
			}
			else {
				int left_contribution = get_split_count(search_path[i].node->left, LeftSide);
				int right_contribution = node->direction == Right ? node->right->left_split_count : 0;
				node->left_split_count = left_contribution + right_contribution;
			}
		}

		for (int i = (int)search_path.size() - 1; i >= 0; i--) {
			Tree<T>* node = search_path[i].node;
			if (is_leaf(node)) {
				node->right_split_count = node->direction == Right ? 0 : 1;
			}
			else {
				auto right_contribution = get_split_count(node->right, RightSide);
				auto left_contribution = node->direction == Left ? node->left->right_split_count : 0;
				node->right_split_count = left_contribution + right_contribution;
			}
		}
	}

	// Find the k nearest in a single tree
	template <class T> T get_k_nearest_single(Tree<T>* tree, int k, SplitSide split_side) {
		while (!is_leaf(tree)) {
			int closerCount = get_split_count(tree, split_side, LEQ);

			if (closerCount <= k) {
				k -= closerCount;
				tree = get_tree_side(tree, split_side, GE);
			}
			else {
				tree = get_tree_side(tree, split_side, LEQ);
			}
		}
		return tree->value;
	}

	// TODO: distance measure could be more generic; instead of directly comparing distance
	// have a function which returns which of the two points is closer.
	template <class T> T get_k_nearest(Tree<T>* tree, T q, int k, std::function<double(T, T)> distance) {
		// From now on, we have a red and a blue tree
		// where the root of blue is leq than the root of red. 

		Tree<T> *red = tree, *blue = tree;
		// Update this whenever necessary to keep track of whch side of the tree is right / left.
		// Red side should be determined based on this value
		SplitSide blue_side = LeftSide;  

		//&& !is_leaf(red) && !is_leaf(blue)
		while (red != nullptr && blue != nullptr) {
			// First, check if orientation of trees is correct
			if (distance(q, blue->value) > distance(q, red->value)) {
				// time to flip
				auto temp = red;
				red = blue;
				blue = temp;
				blue_side = flip(blue_side);
			}
			std::cout << "R/B status after correction: " << std::endl;
			std::cout << "Red " << (blue_side == LeftSide ? "(right): " : "(left): ") << "value: " << red->value << " count: " << get_split_count(red, flip(blue_side)) << std::endl;
			std::cout << "Blue " << (blue_side == RightSide ? "(right): " : "(left): ") << "value: "  << blue->value << " count: " << get_split_count(blue, blue_side) << std::endl;

			// then, we can determine the l measure. no +1 due to leafs only having content
			int blue_leq_count = get_split_count(blue, blue_side, LEQ);
			int red_leq_count = get_split_count(red, flip(blue_side), LEQ);
			int l = blue_leq_count + red_leq_count;
			std::cout << "red_leq: " << red_leq_count << std::endl;
			std::cout << "blue_leq: " << blue_leq_count << std::endl;
			std::cout << "l: " << l << std::endl;
			std::cout << "k: " << k << std::endl;

			// based on l, we now update red and blue
			//if (l == k && blue_leq_count == k) {
			//	std::cout << "using k == l" << std::endl;
			//	return blue->value; // shortcut
			//}
			//else 
			if (l <= k) {
				// recurse on B> and r, as we have no info about R<
				k -= blue_leq_count; //only throw away B< 
				blue = get_tree_side(blue, blue_side, GE);
			}
			else {
				red = get_tree_side(red, flip(blue_side), LEQ);
			}
		}

		if (red == nullptr || blue == nullptr) {
			// a tree was accessed that was not available; this means that we need to continue the
			// search in the remaining tree using the latest k.
			std::cout << (red == nullptr ? "red " : "blue ") << "became nullptr, using other as rt with k=" << k << std::endl;
			auto rt = red == nullptr ? blue : red;
			auto rt_side = red == nullptr ? blue_side : flip(blue_side);

			return get_k_nearest_single(rt, k, rt_side);
		}
		else {
			std::cout << "ERROR: SHOULD NOT OCCUR" << std::endl;
			// tree that is not leaf should be explored
			std::cout << (is_leaf(red) ? "red " : "blue ") << "became leaf, using other as rt with k=" << k << std::endl;
			auto rt = is_leaf(red)  ? blue : red;
			auto rt_side = is_leaf(red) ? blue_side : flip(blue_side);

			return get_k_nearest_single(rt, k, rt_side);
		}
	}
}