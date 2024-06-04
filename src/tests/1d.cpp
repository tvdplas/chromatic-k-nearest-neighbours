#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include "../datastructures/tree.cpp";
#include "../datastructures/1d_mode.cpp";

typedef struct { unsigned int begin; unsigned int end; } IndexRange;

namespace Test {
	// Reference implementation
	// O(n lg n) (sorting entire input array)
	static unsigned int sort_all(std::vector<double>* input, int k, double q) {
		// copy array for in place operation
		std::vector<double> copy = *input;

		auto f = [&q](double x, double y) -> bool { return std::abs(x - q) < std::abs(y - q); };
		std::sort(copy.begin(), copy.end(), f);
		return (unsigned int)k - 1;
	}

	// Slightly smarter reference implementation
	// O(lg n + k lg k) (binary search to find location of q, sort k items on either side)
	static unsigned int sort_k(std::vector<double>* input, int k, double q) {
		// first, binary search
		int i = 0, j = input->size();
		while (i < j) {
			int m = i / 2 + j / 2;
			if ((*input)[m] > q) {
				j = m;
			}
			else {
				i = m + 1;
			}
		}

		int start = std::max(i - k, 0);
		int end = std::min(i + k, (int)input->size() - 1);
		std::vector<double> copy = std::vector<double>(end - start + 1);
		for (int x = start; x <= end; x++) {
			copy[x - start] = (*input)[x];
		}

		auto f = [&q](double x, double y) -> bool { return std::abs(x - q) < std::abs(y - q); };
		std::sort(copy.begin(), copy.end(), f);
		return (unsigned int)k - 1;
	}

	// Get range from the returned
	static IndexRange get_range(std::vector<double>* input, double q, int k, unsigned int index) {
		if ((*input)[index] > q)
			return { index - k + 1, index + 1 };
		else
			return { index, index + k };
	}

	static std::vector<double> generate_sequence(double min, double max, int count, int seed = -1) {
		std::vector<double> items = std::vector<double>(count);

		unsigned int re_seed = seed != -1 ? (unsigned int)seed : std::chrono::system_clock::now().time_since_epoch().count();
		std::uniform_real_distribution<double> rnd(min, max);
		std::default_random_engine re(re_seed);

		for (int i = 0; i < count; i++) {
			items[i] = rnd(re);
		}
		std::sort(items.begin(), items.end());
		return items;
	}

	static void run_1d(int num_runs, int num_items, int num_queries, double min, double max, unsigned int Delta, int k = 0) {
		std::vector<long> generation_times = {},
			tree_build_times = {},
			tree_query_times = {},
			//naive_query_times = {},
			less_naive_query_times = {},
			naive_mode_times = {},
			fast_mode_times = {};


		for (int run_num = 1; run_num <= num_runs; run_num++) {
			std::vector<unsigned int> indexes_tree = std::vector<unsigned int>(num_queries);
			std::vector<unsigned int> indexes_naive = std::vector<unsigned int>(num_queries);

			auto run_start = std::chrono::high_resolution_clock::now();
			
			// instance generation
			auto items = generate_sequence(min, max, num_items);
			auto query_points = generate_sequence(min, max, num_queries);
			auto gen_end = std::chrono::high_resolution_clock::now();
			generation_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(gen_end - run_start).count());
			auto A = DS::generate_colors(num_items, Delta); // imagine that each color is linked to the corresponding index in the tree
			auto mode_preprocessed = DS::preprocess(A);

			// tree build
			auto tree = DS::generate_tree<double>(items);
			auto tree_build_end = std::chrono::high_resolution_clock::now();
			tree_build_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(tree_build_end - gen_end).count());

			// tree query
			auto leq = [](double a, double b) -> bool { return a <= b; };
			auto distance = [](double a, double b) -> double { return std::abs(a - b); };
			for (int i = 0; i < query_points.size(); i++) {
				indexes_tree[i] = DS::query_k_nearest<double>(tree, k, query_points[i], leq, distance);
			}
			auto tree_query_end = std::chrono::high_resolution_clock::now();
			tree_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(tree_query_end - tree_build_end).count());

			// naive queries
		/*	for (auto i = query_points.begin(); i < query_points.end(); i++) {
				sort_all(&items, k, *i);
			}
			auto naive_end = std::chrono::high_resolution_clock::now();
			naive_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(naive_end - tree_query_end).count());*/

			// naive queries
			for (int i = 0; i < query_points.size(); i++) {
				indexes_naive[i] = sort_k(&items, k, query_points[i]);
			}
			auto less_naive_end = std::chrono::high_resolution_clock::now();
			less_naive_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(less_naive_end - tree_query_end).count());

			// Now, do mode queries
			for (int i = 0; i < indexes_tree.size(); i++) {
				auto range = get_range(&items, query_points[i], k, indexes_tree[i]);
				DS::get_mode(mode_preprocessed, range.begin, range.end);
			}
			auto fast_mode_end = std::chrono::high_resolution_clock::now();
			fast_mode_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(fast_mode_end - less_naive_end).count());



			// Now, do mode queries
			for (int i = 0; i < indexes_tree.size(); i++) {
				auto range = get_range(&items, query_points[i], k, indexes_tree[i]);
				DS::get_mode_naive(A, range.begin, range.end);
			}
			auto naive_mode_end = std::chrono::high_resolution_clock::now();
			naive_mode_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(naive_mode_end - fast_mode_end).count());

			DS::free_tree(tree);
		}
		auto avg_gen = std::accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs;
		auto avg_build = std::accumulate(tree_build_times.begin(), tree_build_times.end(), 0) / num_runs;
		auto avg_tree_query = std::accumulate(tree_query_times.begin(), tree_query_times.end(), 0) / num_runs;
		//auto avg_naive_query = std::accumulate(naive_query_times.begin(), naive_query_times.end(), 0) / num_runs;
		auto avg_less_naive = std::accumulate(less_naive_query_times.begin(), less_naive_query_times.end(), 0) / num_runs;
		auto avg_fast_mode = std::accumulate(fast_mode_times.begin(), fast_mode_times.end(), 0) / num_runs;
		auto avg_naive_mode = std::accumulate(naive_mode_times.begin(), naive_mode_times.end(), 0) / num_runs;

		std::cout << "Avg gen time: " << avg_gen << std::endl;
		std::cout << "Avg build time: " << avg_build << std::endl;
		std::cout << "Avg tree range query time: " << avg_tree_query << std::endl;
		//std::cout << "Avg naive query time: " << avg_naive_query << std::endl;
		std::cout << "Avg naive range query time: " << avg_less_naive << std::endl;
		std::cout << "Avg fast mode query time: " << avg_fast_mode << std::endl;
		std::cout << "Avg naive mode query time: " << avg_naive_mode << std::endl;
	}
}