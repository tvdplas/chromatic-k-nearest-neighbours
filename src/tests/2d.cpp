#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include <set>
#include <CGAL/constructions_d.h>
#include "../2D/range_query.cpp"
#include "../2D/mode_query.cpp";

namespace N2D {
	static vec<string> split(string str) {
		vec<string> substrings = {};
		string token;
		istringstream tokenStream(str);

		while (getline(tokenStream, token, ';')) {
			substrings.push_back(token);
		}

		return substrings;
	}


	static vec<pair<Point_d, Color>> read_file(string filename) {
		string text;
		vec<pair<Point_d, Color>> points = {};

		ifstream file(filename);

		while (getline(file, text)) {
			vec<string> args = split(text);

			points.push_back(pair<Point_d, Color>(
				Point_d({ stod(args[0]), stod(args[1]) }), 
				(Color)stoi(args[args.size() - 1])
			));
		}

		file.close();

		return points;
	}

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

		default_random_engine re(chrono::system_clock::now().time_since_epoch().count());
		uniform_real_distribution<double> rnd(0, 1);
		
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

	// As the range tree implementation is already pretty simple, the only thing naiver than this that I could come 
	// up with is the O(n log n) sorting of all points and then taking the kth element
	static NumTy naive_range(vec<Point_d>* locations, Point_d q, int k) {
		vec<NumTy> distances(locations->size());
		for (int i = 0; i < distances.size(); i++) {
			auto dx = abs((*locations)[i].x() - q.x());
			auto dy = abs((*locations)[i].y() - q.y());
			distances[i] = max(dx, dy);
		}
		sort(distances.begin(), distances.end());
		return distances[k];
	}

	// Get mode by querying the radius using a range tree.
	// O(log n + k) complexity.
	static pair<Color, int> naive_mode(Tree rt, Point_d q, NumTy r) {
		Point_d lower({ q.x() - r, q.y() - r }), upper({ q.x() + r, q.y() + r });
		auto points = rt->pointsInRange(lower, upper);
		map<Color, int> candidate_modes;
		for (auto point : points) {
			auto color = point.value();
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

	static void run_2d_single(
		int Q,
		int k,
		int r,
		vec<vec<Point_d>>* pre_gen_points,
		vec<vec<Color>>* pre_gen_colors
	) {
		if (pre_gen_points->size() != pre_gen_colors->size()) return;

		int num_runs = pre_gen_points->size();

		vec<long>
			range_tree_build_times = {},
			range_tree_query_times = {},
			range_naive_query_times = {},
			mode_gen_times = {},
			mode_fast_times = {},
			mode_naive_times = {};

		for (int run_num = 1; run_num <= num_runs; run_num++) {
			auto run_start = chrono::high_resolution_clock::now();

			// generate dataset
			
			auto locations = (*pre_gen_points)[run_num - 1];
			auto colors = (*pre_gen_colors)[run_num - 1];
			vec<NumTy> radii = {};

			// generate tree
			auto tree = generate_tree(&locations, &colors);
			auto sorted_x_pairs = generate_sorted_dim_pairs(&locations, 0);
			auto sorted_y_pairs = generate_sorted_dim_pairs(&locations, 1);
			auto sorted_x_values = get_sorted_dim_values(&sorted_x_pairs);
			auto sorted_y_values = get_sorted_dim_values(&sorted_y_pairs);

			// TODO: fix st entire rectangle can be queried
			auto query_points = generate_locations(
				max(sorted_x_values[1], sorted_y_values[1]),
				min(sorted_x_values[sorted_x_values.size() - 2], sorted_y_values[sorted_y_values.size() - 2]),
				Q
			);
			auto gen_tree_end = chrono::high_resolution_clock::now();

			// perform range queries using quick method
			for (int i = 0; i < Q; i++) {
				auto res = N2D::query_k_nearest(&tree, &sorted_x_values, &sorted_y_values, query_points[i], k); // distance not used for now
			}
			auto range_tree_end = chrono::high_resolution_clock::now();

			// perform range queries using naive method
			for (int i = 0; i < Q; i++) {
				radii.push_back(naive_range(&locations, query_points[i], k));
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
				naive_mode(&tree, query_points[i], radii[i]);
			}
			auto naive_mode_end = chrono::high_resolution_clock::now();


			// push times to array
			range_tree_build_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_tree_end - run_start).count());
			range_tree_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_tree_end - gen_tree_end).count());
			range_naive_query_times.push_back(chrono::duration_cast<chrono::microseconds>(range_naive_end - range_tree_end).count());
			mode_gen_times.push_back(chrono::duration_cast<chrono::microseconds>(gen_mode_end - range_naive_end).count());
			mode_fast_times.push_back(chrono::duration_cast<chrono::microseconds>(fast_mode_end - gen_mode_end).count());
			mode_naive_times.push_back(chrono::duration_cast<chrono::microseconds>(naive_mode_end - fast_mode_end).count());
		}

		// finalize times
		auto avg_range_tree_build = ((long)(accumulate(range_tree_build_times.begin(), range_tree_build_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_tree_query = ((long)(accumulate(range_tree_query_times.begin(), range_tree_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_range_naive_query = ((long)(accumulate(range_naive_query_times.begin(), range_naive_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_gen = ((long)(accumulate(mode_gen_times.begin(), mode_gen_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_fast = ((long)(accumulate(mode_fast_times.begin(), mode_fast_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_mode_naive = ((long)(accumulate(mode_naive_times.begin(), mode_naive_times.end(), 0) / num_runs / 100.0)) / 10.0;

		// print to console
		cout << fixed << setprecision(1);
		cout
			<< avg_range_tree_build << " & "
			<< avg_range_tree_query << " & "
			<< avg_range_naive_query << " & "
			<< avg_mode_gen << " & "
			<< avg_mode_fast << " & "
			<< avg_mode_naive << " & "
			<< "\\\\" << endl;
	}

	static void run_2d_single(int num_runs, int N, int k, int r, NumTy min, NumTy max, int Q, int delta, double gamma, double alpha) {
		vec<vec<Point_d>> pre_gen_points = {};
		vec<vec<Color>> pre_gen_colors = {};
		
		for (int i = 0; i < num_runs; i++) {
			auto dataset = generate_sequence(min, max, N, delta, gamma, alpha);
			pre_gen_points.push_back(dataset.first);
			pre_gen_colors.push_back(dataset.second);
		}

		run_2d_single(Q, k, r, &pre_gen_points, &pre_gen_colors);
	}

	static void run_2d_real() {
		int Q = 1000;
		string rel_dir = "..\\data\\osm\\";

		vec<string> files = {
			"1.points",
			"2.points",
			"3.points",
			"4.points",
			"5.points",
			"6.points",
			"7.points",
			"8.points",
			"9.points",
			"10.points",
		};

		vec<vec<pair<Point_d, Color>>> bases = {};
		vec<int> sizes = {};
		for (int i = 0; i < files.size(); i++) {
			auto data = read_file(rel_dir + files[i]);
			bases.push_back(data);
			sizes.push_back(data.size());
		}

		auto min_size = *min_element(sizes.begin(), sizes.end());

		vec<double> frac_n = { 0.01, 0.05, 0.1, 0.25, 0.5, 0.75, 1 };
		vec<int> ks = { 25 , 50, 75, 100, 250, 500, 750, 1000, 1500, 2000 };
		vec<int> rs = { 10 };

		for (int i_n = 0; i_n < frac_n.size(); i_n++) {
			for (int i_k = 0; i_k < ks.size() && ks[i_k] < min_size * frac_n[i_n]; i_k++) {
				for (int i_r = 0; i_r < rs.size(); i_r++) {
					// Subsample each base
					vec<vec<Point_d>> sampled_locations = {};
					vec<vec<Color>> sampled_colors = {};
					for (int i = 0; i < bases.size(); i++) {
						auto s = random_split(&bases[i], frac_n[i_n] * bases[i].size());
						auto considered = s.first;

						vec<Point_d> locations = {};
						vec<Color> colors = {};
						for (int j = 0; j < considered.size(); j++) {
							locations.push_back(considered[j].first);
							colors.push_back(considered[j].second);
						}
						sampled_locations.push_back(locations);
						sampled_colors.push_back(colors);
					}

					cout << defaultfloat;
					cout
						<< "2D-MAP-" << (int)ks[i_k]
						<< "-" << (int)rs[i_r]
						<< "-" << (int)(100 * frac_n[i_n])
						<< " & " << (int)(frac_n[i_n] * min_size) << " & ";

					run_2d_single(Q, ks[i_k], 10, &sampled_locations, &sampled_colors);
				}
			}
			cout << "\\hline \\\\" << endl;;
		}
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
					cout << defaultfloat;
					cout 
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