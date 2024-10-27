#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include <set>
#include "../2D/range_query.cpp"
#include "../2D/mode_query.cpp";

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

	static vec<Point_2> convert_point_d_to_point_2(vec<Point_d>* points) {
		vec<Point_2> output(points->size());
		for (int i = 0; i < points->size(); i++) {
			auto p = (*points)[i];
			output[i] = Point_2(p.x(), p.y());
		}
		return output;
	}

	static pair<vec<Point_d>, vec<Color>> generate_sequence(NumTy min, NumTy max, int count, int delta, double gamma, double alpha) {
		vec<Point_d> locations = generate_locations(min, max, count);
		vec<Color> colors(count);
		vec<Point_d> gamma_locations(gamma);
		vec<Color> gamma_colors(count);

		std::default_random_engine re(chrono::system_clock::now().time_since_epoch().count());
		std::uniform_real_distribution<double> rnd(0, 1);
		
		for (int i = 0; i < count; i++) {
			gamma_colors[i] = (Color)delta;
			colors[i] = (Color)delta;
		}
		for (int i = 0; i < gamma; i++) {
			Color c = (Color)(rnd(re) * delta);
			int index = rnd(re) * count;
			gamma_colors[index] = c;
			gamma_locations[i] = locations[index];
		}

		auto gamma_sorted_x_pairs = generate_sorted_dim_pairs(&gamma_locations, 0);
		auto gamma_sorted_y_pairs = generate_sorted_dim_pairs(&gamma_locations, 1);

		for (int i = 0; i < count; i++) {
			if (rnd(re) >= alpha) {
				// use uniform sample
				Color c = (Color)(rnd(re) * delta);
				colors[i] = c;
			}
			else {
				// find closest in gamma list
				Color closest = (Color)delta;
				auto f = [](pair_ni a, pair_ni b) { return a.first - b.first;  };
				auto x_index = upper_bound(gamma_sorted_x_pairs.begin(), gamma_sorted_x_pairs.end(), pair_ni(locations[i].x(), -1), f) - gamma_sorted_x_pairs.begin();
				auto y_index = upper_bound(gamma_sorted_y_pairs.begin(), gamma_sorted_y_pairs.end(), pair_ni(locations[i].y(), -1), f) - gamma_sorted_y_pairs.begin();

				NumTy min_distance = x_index - 1 < gamma_sorted_x_pairs.size() ? abs(gamma_sorted_x_pairs[x_index - 1].first - locations[i].x()) : numeric_limits<NumTy>::max();
				Color gamma_index = x_index - 1< gamma_sorted_x_pairs.size() ? gamma_sorted_x_pairs[x_index - 1].second : delta;
				if (x_index < gamma_sorted_x_pairs.size() && abs(gamma_sorted_x_pairs[x_index].first - locations[i].x()) < min_distance) {
					min_distance = abs(gamma_sorted_x_pairs[x_index].first - locations[i].x());
					gamma_index = gamma_sorted_x_pairs[x_index].second;
				}
				if (y_index - 1 < gamma_sorted_y_pairs.size() && abs(gamma_sorted_y_pairs[y_index - 1].first - locations[i].y()) < min_distance) {
					min_distance = abs(gamma_sorted_y_pairs[y_index - 1].first - locations[i].y());
					gamma_index = gamma_sorted_y_pairs[y_index - 1].second;
				}
				if (y_index < gamma_sorted_y_pairs.size() && abs(gamma_sorted_y_pairs[y_index].first - locations[i].y()) < min_distance) {
					min_distance = abs(gamma_sorted_y_pairs[y_index].first - locations[i].y());
					gamma_index = gamma_sorted_y_pairs[y_index].second;
				}

				colors[i] = gamma_colors[gamma_index];
			}
		}


		return pair<vec<Point_d>, vec<Color>>(locations, colors);
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


	static void run_2d_single(int num_runs, int N, int k, int r, NumTy min, NumTy max, int Q, int delta, double gamma, double alpha) {
		vec<long>
			generation_times = {},
			range_tree_build_times = {},
			range_tree_query_times = {},
			range_naive_query_times = {},
			mode_gen_times = {},
			mode_fast_times = {},
			mode_naive_times = {};

		for (int run_num = 1; run_num <= num_runs; run_num++) {
			auto run_start = chrono::high_resolution_clock::now();

			// generate dataset
			auto dataset = generate_sequence(min, max, N, delta, gamma, alpha);
			auto locations = dataset.first;
			auto colors = dataset.second;
			auto query_points = generate_locations(min, max, Q);
			auto gen_end = chrono::high_resolution_clock::now();
			vec<NumTy> radii = {};

			// generate tree
			auto tree = generate_tree(&locations, &colors);
			auto sorted_x_pairs = generate_sorted_dim_pairs(&locations, 0);
			auto sorted_y_pairs = generate_sorted_dim_pairs(&locations, 1);
			auto sorted_x_values = get_sorted_dim_values(&sorted_x_pairs);
			auto sorted_y_values = get_sorted_dim_values(&sorted_y_pairs);
			auto gen_tree_end = chrono::high_resolution_clock::now();

			// perform range queries using quick method
			for (int i = 0; i < Q; i++) {
				auto res = N2D::query_k_nearest(&tree, &sorted_x_values, &sorted_y_values, query_points[i], k); // distance not used for now
			}
			auto range_tree_end = chrono::high_resolution_clock::now();

			// perform range queries using naive method
			for (int i = 0; i < Q; i++) {
				radii.push_back(naive_range(&sorted_x_pairs, &sorted_y_pairs, query_points[i], k));
			}
			auto range_naive_end = chrono::high_resolution_clock::now();

			vec<Point_2> mode_points = convert_point_d_to_point_2(&locations);
			ModeData md = preprocess_mode(&mode_points, &colors, r);
			Trapezoid_pl tpl(md.arr);
			uniform_real_distribution<NumTy> rnd_pos(md.lower.x(), md.upper.x());
			default_random_engine re(chrono::system_clock::now().time_since_epoch().count());
			auto gen_mode_end = chrono::high_resolution_clock::now();

			// perform mode queries using quick method
			for (int i = 0; i < Q; i++) {
				// TODO: replace with actual dual
				Point_2 query(rnd_pos(re), rnd_pos(re));
				query_arrangement(&md, &tpl, &colors, query);
			}
			auto fast_mode_end = chrono::high_resolution_clock::now();

			// perform mode queries using naive method
			for (int i = 0; i < Q; i++) {
				naive_mode(&sorted_x_pairs, &sorted_y_pairs, &colors, query_points[i], radii[i]);
			}
			auto naive_mode_end = chrono::high_resolution_clock::now();


			// push times to array
			generation_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_end - run_start).count());
			range_tree_build_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_tree_end - gen_end).count());
			range_tree_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_tree_end - gen_tree_end).count());
			range_naive_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_naive_end - range_tree_end).count());
			mode_gen_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_mode_end - range_naive_end).count());
			mode_fast_times.push_back(chrono::duration_cast<chrono::microseconds>(fast_mode_end - gen_mode_end).count());
			mode_naive_times.push_back(chrono::duration_cast<chrono::microseconds>(naive_mode_end - fast_mode_end).count());
		}

		// finalize times
		auto avg_gen = ((long)(accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_build = ((long)(accumulate(range_tree_build_times.begin(), range_tree_build_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_query = ((long)(accumulate(range_tree_query_times.begin(), range_tree_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_naive_query = ((long)(accumulate(range_naive_query_times.begin(), range_naive_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_gen = ((long)(accumulate(mode_gen_times.begin(), mode_gen_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_fast = ((long)(accumulate(mode_fast_times.begin(), mode_fast_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_naive = ((long)(accumulate(mode_naive_times.begin(), mode_naive_times.end(), 0) / num_runs / 100.0)) / 10.0;

		// print to console
		std::cout << fixed << setprecision(1);
		std::cout 
			<< avg_gen << " & " 
			<< avg_range_tree_build << " & " 
			<< avg_range_tree_query << " & " 
			<< avg_range_naive_query << " & "
			<< avg_mode_gen << " & "
			<< avg_mode_fast << " & "
			<< avg_mode_naive << " & "
			<< "\\\\" << endl;
	}

	static void run_2d_generated() {
		int num_runs = 5;
		vec<vec<NumTy>> scenarios = {
			// N, min, max, Q, Delta, gamma, alpha, 
			{ 1e3, -5e4, 5e4, 1e3, 20, 0, 0},
			{ 1e4, -5e4, 5e4, 1e3, 20, 0, 0},
			{ 1e5, -5e4, 5e4, 1e3, 20, 0, 0},
			{ 1e3, -5e4, 5e4, 1e3, 100, 0, 0},
			{ 1e4, -5e4, 5e4, 1e3, 100, 0, 0},
			{ 1e5, -5e4, 5e4, 1e3, 100, 0, 0},
			{ 1e3, -5e4, 5e4, 1e3, 20, 30, 0.95},
			{ 1e4, -5e4, 5e4, 1e3, 20, 30, 0.95},
			{ 1e5, -5e4, 5e4, 1e3, 20, 30, 0.95},
			{ 1e3, -5e4, 5e4, 1e3, 100, 150, 0.95},
			{ 1e4, -5e4, 5e4, 1e3, 100, 150, 0.95},
			{ 1e5, -5e4, 5e4, 1e3, 100, 150, 0.95},
		};

		vec<int> rs = { 10 };

		vec<int> ks = { 25 , 50, 75, 100, 250, 500, 750, 1000, 1500, 2000 }; // k = 1000 only for N > 1000;
		
		for (int sIndex = 0; sIndex < scenarios.size(); sIndex++) {
			auto scenario = scenarios[sIndex];
			for (int r_index = 0; r_index < rs.size(); r_index++) {
				for (int k_index = 0; k_index < ks.size(); k_index++) {
					if (ks[k_index] >= scenario[0]) continue; // skip due to k >= n
					std::cout << defaultfloat;
					std::cout 
						<< "2D-" << (int)scenario[0] 
						<< "-" << rs[r_index]
						<< "-" << ks[k_index] 
						<< "-" << (int)scenario[4]
						<< "-" << scenario[5]
						<< "-" << scenario[6]
						<< " & ";
				
					run_2d_single(
						num_runs, 
						(int)scenario[0], 
						ks[k_index], 
						rs[r_index],
						(int)scenario[1], 
						(int)scenario[2], 
						(int)scenario[3],
						(int)scenario[4],
						scenario[5],
						scenario[6]
					);
				}
				std::cout << "\\hline" << endl;
			}
		}
	}
}