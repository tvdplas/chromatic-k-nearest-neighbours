#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include "../datastructures/tree.cpp";

namespace Test {
	// Reference implementation
	// O(n lg n) (sorting entire input array)
	static double sort_all(std::vector<double>* input, int k, double q) {
		// copy array for in place operation
		std::vector<double> copy = *input;

		auto f = [&q](double x, double y) -> bool { return std::abs(x - q) < std::abs(y - q); };
		std::sort(copy.begin(), copy.end(), f);
		return copy[k - 1];
	}

	// Slightly smarter reference implementation
	// O(lg n + k lg k) (binary search to find location of q, sort k items on either side)
	static double sort_k(std::vector<double>* input, int k, double q) {
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
		return copy[k - 1];
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

	static void run_1d_range_finding(int num_runs, int num_items, int num_queries, double min, double max, int k = 0) {
		std::vector<long> generation_times = {},
			tree_build_times = {},
			tree_query_times = {},
			naive_query_times = {},
			less_naive_query_times = {};


		for (int run_num = 1; run_num <= num_runs; run_num++) {
			auto run_start = std::chrono::high_resolution_clock::now();
			
			// instance generation
			auto items = generate_sequence(min, max, num_items);
			auto query_points = generate_sequence(min, max, num_queries);
			auto gen_end = std::chrono::high_resolution_clock::now();
			generation_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(gen_end - run_start).count());

			// tree build
			auto tree = DS::generate_tree<double>(items);
			auto tree_build_end = std::chrono::high_resolution_clock::now();
			tree_build_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(tree_build_end - gen_end).count());

			// tree query
			auto leq = [](double a, double b) -> bool { return a <= b; };
			auto distance = [](double a, double b) -> double { return std::abs(a - b); };
			for (auto i = query_points.begin(); i < query_points.end(); i++) {
				DS::query_k_nearest<double>(tree, k, *i, leq, distance);
			}
			auto tree_query_end = std::chrono::high_resolution_clock::now();
			tree_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(tree_query_end - tree_build_end).count());

			// naive queries
			for (auto i = query_points.begin(); i < query_points.end(); i++) {
				sort_all(&items, k, *i);
			}
			auto naive_end = std::chrono::high_resolution_clock::now();
			naive_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(naive_end - tree_query_end).count());

			// naive queries
			for (auto i = query_points.begin(); i < query_points.end(); i++) {
				sort_k(&items, k, *i);
			}
			auto less_naive_end = std::chrono::high_resolution_clock::now();
			less_naive_query_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(less_naive_end - naive_end).count());
		}
		auto avg_gen = std::accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs;
		auto avg_build = std::accumulate(tree_build_times.begin(), tree_build_times.end(), 0) / num_runs;
		auto avg_tree_query = std::accumulate(tree_query_times.begin(), tree_query_times.end(), 0) / num_runs;
		auto avg_naive_query = std::accumulate(naive_query_times.begin(), naive_query_times.end(), 0) / num_runs;
		auto avg_less_naive = std::accumulate(less_naive_query_times.begin(), less_naive_query_times.end(), 0) / num_runs;

		std::cout << "Avg gen time: " << avg_gen << std::endl;
		std::cout << "Avg build time: " << avg_build << std::endl;
		std::cout << "Avg tree query time: " << avg_tree_query << std::endl;
		std::cout << "Avg naive query time: " << avg_naive_query << std::endl;
		std::cout << "Avg less naive query time: " << avg_less_naive << std::endl;
	}
}