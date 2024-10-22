#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include <set>
#include "../2D/range_query.cpp"
#include "../2D/mode_query.cpp"

namespace N2D {
	static vec<Point_d> generate_locations(NumTy min, NumTy max, int count) {
		vec<Point_d> positions(count);

		unsigned int re_seed = chrono::system_clock::now().time_since_epoch().count();
		uniform_real_distribution<NumTy> rnd_pos(min, max);
		default_random_engine re(re_seed);		

		for (int i = 0; i < count; i++) {
			NumTy x = rnd_pos(re);
			NumTy y = rnd_pos(re);
			positions[i] = Point_d({ x, y });
		}
		return positions;
	}

	static vec<Point_2> generate_locations_mode(NumTy min, NumTy max, int count) {
		auto locations = generate_locations(min, max, count);
		vec<Point_2> mode_locations(locations.size());
		for (int i = 0; i < locations.size(); i++) {
			mode_locations[i] = Point_2(locations[i].x(), locations[i].y());
		}
		return mode_locations;
	}

	static vec<Color> generate_colors(int count) {
		// TODO: implement 
		vec<Color> colors(count);
		
		for (int i = 0; i < count; i++) {
			colors[i] = 0;
		}

		return colors;
	}

	// returns radius of D_k(q)
	static NumTy naive_range(vec<pair_ni>* sorted_x_pairs, vec<pair_ni>* sorted_y_pairs, Point_d q, int k) {
		// find q within the sorted x and y arrays;
		auto comp = [](pair_ni pair, pair_ni val) {
			return pair.first < val.first;
		};
		auto q_x = q.x();
		auto q_y = q.y();
		int q_x_index = distance(sorted_x_pairs->begin(), upper_bound(sorted_x_pairs->begin(), sorted_x_pairs->end(), pair_ni(q.x(), -1), comp));
		int q_y_index = distance(sorted_y_pairs->begin(), upper_bound(sorted_y_pairs->begin(), sorted_y_pairs->end(), pair_ni(q.y(), -1), comp));

		set<int> visited_points;
		int x_lower = q_x_index - 1,
			x_upper = q_x_index,
			y_lower = q_y_index - 1,
			y_upper = q_y_index;
		NumTy max_distance = numeric_limits<NumTy>::max();
		NumTy curr_min_distance = max_distance;
		

		// now step k times
		for (int i = 0; i < k; i++) {
			// find distances for each possible step that has not yet been visited
			NumTy d_x_lower, d_x_upper, d_y_lower, d_y_upper;
			while (true) {
				if (x_lower <= 0) { d_x_lower = max_distance; break; }
				if (visited_points.count((*sorted_x_pairs)[x_lower].second)) { x_lower--; continue; }
				d_x_lower = abs((*sorted_x_pairs)[x_lower].first - q.x()); break;
			}
			while (true) {
				if (x_upper >= sorted_x_pairs->size() - 1) { d_x_upper = max_distance; break; }
				if (visited_points.count((*sorted_x_pairs)[x_upper].second)) { x_upper++; continue; }
				d_x_upper = abs((*sorted_x_pairs)[x_upper].first - q.x()); break;
			}
			while (true) {
				if (y_lower <= 0) { d_y_lower = max_distance; break; }
				if (visited_points.count((*sorted_y_pairs)[y_lower].second)) { y_lower--; continue; }
				d_y_lower = abs((*sorted_y_pairs)[y_lower].first - q.y()); break;
			}
			while (true) {
				if (y_upper >= sorted_y_pairs->size() - 1) { d_y_upper = max_distance; break; }
				if (visited_points.count((*sorted_y_pairs)[y_upper].second)) { y_upper++; continue; }
				d_y_upper = abs((*sorted_y_pairs)[y_upper].first - q.y()); break;
			}
			
			// select minimum distance
			curr_min_distance = min({ d_x_lower, d_x_upper, d_y_lower, d_y_upper });
			if (d_x_lower == curr_min_distance) {
				visited_points.insert((*sorted_x_pairs)[x_lower].second);
				x_lower--;
			}
			else if (d_x_upper == curr_min_distance) {
				visited_points.insert((*sorted_x_pairs)[x_upper].second);
				x_upper++;
			}
			else if (d_y_lower == curr_min_distance) {
				visited_points.insert((*sorted_x_pairs)[y_lower].second);
				y_lower--;
			}
			else {
				visited_points.insert((*sorted_x_pairs)[y_upper].second);
				y_upper++;
			}
		}

		return curr_min_distance;
	}

	static pair<Color, int> naive_mode(vec<pair_ni>* sorted_x_pairs, vec<pair_ni>* sorted_y_pairs, vec<Color>* colors, Point_d q, NumTy r) {
		auto comp = [](pair_ni a, pair_ni b) { return a.first <= b.first; };
		auto x_start = lower_bound(sorted_x_pairs->begin(), sorted_x_pairs->end(), pair_ni(q.x() - r, -1), comp);
		auto x_end = lower_bound(sorted_x_pairs->begin(), sorted_x_pairs->end(), pair_ni(q.x() + r, -1), comp);
		auto y_start = lower_bound(sorted_y_pairs->begin(), sorted_y_pairs->end(), pair_ni(q.y() - r, -1), comp);
		auto y_end = lower_bound(sorted_y_pairs->begin(), sorted_y_pairs->end(), pair_ni(q.y() - r, -1), comp);

		set<int> indexes;
		for (auto x = x_start; x < x_end; x++) indexes.insert(x->second);
		for (auto y = y_start; y < y_end; y++) indexes.insert(y->second);
		map<Color, int> candidate_modes;
		for (int index : indexes) {
			auto color = (*colors)[index];
			if (candidate_modes.count(color)) candidate_modes[color]++;
			else candidate_modes[color] = 1;
		}
		
		auto mode = *max_element(
			begin(candidate_modes),
			end(candidate_modes),
			[](const pair<int, int> a, const pair<int, int> b) { return a.second < b.second; }
		);
		return mode;
	}


	static void run_2d_single(int num_runs, int N, int k, NumTy min, NumTy max, int Q) {
		vec<long>
			generation_times = {},
			range_tree_build_times = {},
			range_tree_query_times = {},
			range_naive_query_times = {},
			mode_fast_mode_times = {},
			mode_naive_times = {};
		vec<double> num_bin_searches = {};

		for (int run_num = 1; run_num <= num_runs; run_num++) {
			auto run_start = chrono::high_resolution_clock::now();

			// generate dataset
			auto locations = generate_locations(min, max, N);
			auto colors = generate_colors(N);
			auto query_points = generate_locations(min, max, Q);
			auto gen_end = chrono::high_resolution_clock::now();
			vec<int> num_bin_searches_run = {};

			// generate tree
			auto tree = generate_tree(&locations, &colors);
			auto sorted_x_pairs = generate_sorted_dim_pairs(&locations, 0);
			auto sorted_y_pairs = generate_sorted_dim_pairs(&locations, 1);
			auto sorted_x_values = get_sorted_dim_values(&sorted_x_pairs);
			auto sorted_y_values = get_sorted_dim_values(&sorted_y_pairs);
			auto gen_tree_end = chrono::high_resolution_clock::now();

			// perform queries using range tree
			for (int i = 0; i < Q; i++) {
				auto res = N2D::query_k_nearest(&tree, &sorted_x_values, &sorted_y_values, query_points[i], k); // distance not used for now
				num_bin_searches_run.push_back(res.second);
			}
			auto range_tree_end = chrono::high_resolution_clock::now();

			for (int i = 0; i < Q; i++) {
				naive_range(&sorted_x_pairs, &sorted_y_pairs, query_points[i], k);
			}
			auto range_naive_end = chrono::high_resolution_clock::now();

			// push times to array
			generation_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_end - run_start).count());
			range_tree_build_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_tree_end - gen_end).count());
			range_tree_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_tree_end - gen_tree_end).count());
			range_naive_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_naive_end - range_tree_end).count());

			// push avg num of bin searches
			num_bin_searches.push_back((double)(accumulate(num_bin_searches_run.begin(), num_bin_searches_run.end(), 0.0)) / (double)Q);
		}

		// finalize times
		auto avg_gen = ((long)(accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_build = ((long)(accumulate(range_tree_build_times.begin(), range_tree_build_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_query = ((long)(accumulate(range_tree_query_times.begin(), range_tree_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_naive_query = ((long)(accumulate(range_naive_query_times.begin(), range_naive_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_bin_searches = ((double)accumulate(num_bin_searches.begin(), num_bin_searches.end(), 0.0) / (double)num_runs);

		// print to console
		cout << avg_gen << " & " << avg_range_tree_build << " & " << avg_range_tree_query << " & " << avg_range_naive_query << "\\\\" << endl;
	}

	static void run_2d_generated() {
		int num_runs = 10;
		vec<vec<NumTy>> scenarios = {
			// N, min, max, Q  ... TODO
			{ 1e3, -5e4, 5e4, 1e3},
			{ 1e4, -5e4, 5e4, 1e3},
			{ 1e5, -5e4, 5e4, 1e3},
		};

		vec<int> ks = { 25, 50, 75, 100, 250, 500, 750, 1000, 1500, 2000 }; // k = 1000 only for N > 1000;
		
		for (int sIndex = 0; sIndex < scenarios.size(); sIndex++) {
			auto scenario = scenarios[sIndex];
			for (int kIndex = 0; kIndex < ks.size(); kIndex++) {
				if (ks[kIndex] >= scenario[0]) continue; // skip due to k >= n
				cout << "2D-" << scenario[0] << "-" << ks[kIndex] << " & ";
				
				run_2d_single(num_runs, (int)scenario[0], ks[kIndex], (int)scenario[1], (int)scenario[2], (int)scenario[3]);
			}
			cout << "\\hline" << endl;
		}
	}
}