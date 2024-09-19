#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include "../2D/range_query.cpp"



namespace N2D {
	static std::vector<Point_d> generate_locations(NumTy min, NumTy max, int count) {
		std::vector<Point_d> positions(count);

		unsigned int re_seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::uniform_real_distribution<NumTy> rnd_pos(min, max);
		std::default_random_engine re(re_seed);		

		for (int i = 0; i < count; i++) {
			NumTy x = rnd_pos(re);
			NumTy y = rnd_pos(re);
			positions[i] = Point_d({ x, y });
		}
		return positions;
	}

	static std::vector<Color> generate_colors(int count) {
		// TODO: implement 
		std::vector<Color> colors(count);
		
		for (int i = 0; i < count; i++) {
			colors[i] = 0;
		}

		return colors;
	}

	static void run_2d_single(int num_runs, int N, int k, NumTy min, NumTy max, int Q) {
		std::vector<long>
			generation_times = {},
			range_tree_build_times = {},
			range_tree_query_times = {},
			range_naive_query_times = {},
			mode_fast_mode_times = {},
			mode_naive_times = {};

		for (int run_num = 1; run_num <= num_runs; run_num++) {
			auto run_start = std::chrono::high_resolution_clock::now();

			// generate dataset
			auto locations = generate_locations(min, max, N);
			auto colors = generate_colors(N);
			auto query_points = generate_locations(min, max, Q);
			auto gen_end = std::chrono::high_resolution_clock::now();

			// generate tree
			auto tree = generate_tree(&locations, &colors);
			auto sorted_x = generate_sorted_dim(&locations, 0);
			auto sorted_y = generate_sorted_dim(&locations, 1);
			auto gen_tree_end = std::chrono::high_resolution_clock::now();

			// perform queries
			for (int i = 0; i < Q; i++) {
				auto distance = N2D::query_k_nearest(&tree, &sorted_x, &sorted_y, query_points[i], k); // distance not used for now
			}
			auto range_tree_end = std::chrono::high_resolution_clock::now();

			// push times to array
			generation_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(gen_end - run_start).count());
			range_tree_build_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(gen_tree_end - gen_end).count());
			range_tree_query_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(range_tree_end - gen_tree_end).count());
		}

		// finalize times
		auto avg_gen = ((long)(std::accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_build = ((long)(std::accumulate(range_tree_build_times.begin(), range_tree_build_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_query = ((long)(std::accumulate(range_tree_query_times.begin(), range_tree_query_times.end(), 0) / num_runs / 100.0)) / 10.0;

		// print to console
		std::cout << avg_gen << " & " << avg_range_tree_build << " & " << avg_range_tree_query << "\\\\" << std::endl;
	}

	static void run_2d_generated() {
		int num_runs = 10;
		std::vector<std::vector<NumTy>> scenarios = {
			// N, min, max, Q  ... TODO
			{ 1e3, -5e4, 5e4, 1e3},
			{ 1e4, -5e4, 5e4, 1e3},
			{ 1e5, -5e4, 5e4, 1e3},
		};

		std::vector<int> ks = { 10, 25, 50, 75, 100, 250, 500, 750, 1000, 1500, 2000 }; // k = 1000 only for N > 1000;
		
		for (int sIndex = 0; sIndex < scenarios.size(); sIndex++) {
			auto scenario = scenarios[sIndex];
			for (int kIndex = 0; kIndex < ks.size(); kIndex++) {
				if (ks[kIndex] >= scenario[0]) continue; // skip due to k >= n
				std::cout << "2D-" << scenario[0] << "-" << ks[kIndex] << " & ";
				
				run_2d_single(num_runs, (int)scenario[0], ks[kIndex], (int)scenario[1], (int)scenario[2], (int)scenario[3]);
			}
			std::cout << "\\hline" << std::endl;
		}
	}
}