#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <chrono> 
#include <numeric>
#include <string>
#include <sstream>
#include <fstream> 
#include <iomanip>
#include "../1D/range_query.cpp";
#include "../1D/mode_query.cpp";

typedef struct { unsigned int begin; unsigned int end; } IndexRange;
typedef struct { double pos; Color color; } Point;

namespace Test {
	static std::vector<std::string> split(std::string str) {
		std::vector<std::string> substrings = {};
		std::string token;
		std::istringstream tokenStream(str);

		while (std::getline(tokenStream, token, ';')) {
			substrings.push_back(token);
		}

		return substrings;
	}


	static std::vector<Point> read_file(std::string filename, int dim) {
		std::string text;
		std::vector<Point> points = {};

		std::ifstream file(filename);

		// Use a while loop together with the getline() function to read the file line by line
		while (std::getline(file, text)) {
			// determine dimensionality of line
			std::vector<std::string> args = split(text);
			int dims = args.size() - 1;

			points.push_back({ std::stod(args[dim]), (Color)std::stoi(args[args.size() - 1]) });
		}

		file.close();

		auto compare = [](const Point &a, const Point &b)
		{
			return a.pos < b.pos;
		};
		std::sort(points.begin(), points.end(), compare);

		return points;
	}

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
			return { index + 1 - k, index + 1 };
		else
			return { index, index + k };
	}

	static std::vector<double> generate_positions(double min, double max, int count) {
		std::vector<double> positions = std::vector<double>(count);

		unsigned int re_seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::uniform_real_distribution<double> rnd_pos(min, max);
		std::default_random_engine re(re_seed);

		for (int i = 0; i < count; i++) {
			positions[i] = rnd_pos(re);
		}
		std::sort(positions.begin(), positions.end());
		return positions;
	}

	static std::vector<Point> generate_sequence(double min, double max, int count, int Delta, double gamma, double alpha) {
		// First, generate positions
		std::vector<double> positions = generate_positions(min, max, count);

		unsigned int re_seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine re(re_seed);

		// Then, colors
		std::uniform_real_distribution<double> rnd(0, 1);

		// Fix gamma points
		std::vector<Color> gamma_colors = std::vector<Color>(count);
		std::vector<Color> colors = std::vector<Color>(count);
		for (int i = 0; i < count; i++) {
			gamma_colors[i] = (Color)Delta;
			colors[i] = (Color)Delta;
		}
		for (int i = 0; i < gamma; i++) {
			Color c = (Color)(rnd(re) * Delta);
			int index = rnd(re) * count;
			gamma_colors[index] = c;
		}

		// Then, fill in rest
		for (int i = 0; i < count; i++) {
			if (rnd(re) >= alpha) {
				// use uniform sample
				Color c = (Color)(rnd(re) * Delta);
				colors[i] = c;
			}
			else {
				// find closest in gamma list, otherwise fall back to uniform
				// O(n^2) implementation, but since this is setup doesn't really matter
				Color closest = (Color)Delta;
				double distance = max - min + 1; 
				for (int j = 0; j < count; j++) {
					if (gamma_colors[j] != (Color)Delta && abs(positions[i] - positions[j]) < distance) {
						distance = abs(positions[i] - positions[j]);
						closest = (Color)gamma_colors[j];
					}
				}
				colors[i] = closest;
			}
		}

		// Lastly, combine the two
		std::vector<Point> points = std::vector<Point>(count);
		for (int i = 0; i < count; i++) {
			points[i] = { positions[i], (Color)colors[i] };
		}
		return points;
	}

	static void run_1d_single(
		int num_queries,
		int k,
		std::vector<std::vector<Point>>* pre_gen_points
	) {
		int num_runs = pre_gen_points->size();

		std::vector<long> generation_times = {},
			tree_build_times = {},
			tree_query_times = {},
			less_naive_query_times = {},
			naive_mode_times = {},
			fast_mode_times = {};

		for (int run_num = 1; run_num <= num_runs; run_num++) {
			std::vector<unsigned int> indexes_tree = std::vector<unsigned int>(num_queries);
			std::vector<unsigned int> indexes_naive = std::vector<unsigned int>(num_queries);

			auto run_start = std::chrono::high_resolution_clock::now();

			// instance generation
			auto points = (*pre_gen_points)[run_num - 1];
			std::vector<double> positions = std::vector<double>(points.size());
			std::vector<Color> A = std::vector<Color>(points.size());
			for (int i = 0; i < points.size(); i++) {
				positions[i] = points[i].pos;
				A[i] = points[i].color;
			}

			double min = *std::min_element(positions.begin(), positions.end());
			double max = *std::max_element(positions.begin(), positions.end());

			auto query_points = generate_positions(min, max, num_queries);
			auto mode_preprocessed = DS::preprocess(A);
			auto gen_end = std::chrono::high_resolution_clock::now();
			generation_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(gen_end - run_start).count());

			// tree build
			auto tree = DS::generate_tree<double>(positions);
			auto tree_build_end = std::chrono::high_resolution_clock::now();
			tree_build_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(tree_build_end - gen_end).count());

			// tree query
			auto leq = [](double a, double b) -> bool { return a <= b; };
			auto distance = [](double a, double b) -> double { return std::abs(a - b); };
			for (int i = 0; i < query_points.size(); i++) {
				indexes_tree[i] = DS::query_k_nearest<double>(tree, k, query_points[i], leq, distance);
			}
			auto tree_query_end = std::chrono::high_resolution_clock::now();
			tree_query_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(tree_query_end - tree_build_end).count());

			// naive queries
			for (int i = 0; i < query_points.size(); i++) {
				indexes_naive[i] = sort_k(&positions, k, query_points[i]);
			}
			auto less_naive_end = std::chrono::high_resolution_clock::now();
			less_naive_query_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(less_naive_end - tree_query_end).count());

			// Now, do mode queries
			for (int i = 0; i < indexes_tree.size(); i++) {
				auto range = get_range(&positions, query_points[i], k, indexes_tree[i]);
				DS::get_mode(mode_preprocessed, range.begin, range.end);
			}
			auto fast_mode_end = std::chrono::high_resolution_clock::now();
			fast_mode_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(fast_mode_end - less_naive_end).count());



			// Now, do mode queries
			for (int i = 0; i < indexes_tree.size(); i++) {
				auto range = get_range(&positions, query_points[i], k, indexes_tree[i]);
				DS::get_mode_naive(&A, range.begin, range.end);
			}
			auto naive_mode_end = std::chrono::high_resolution_clock::now();
			naive_mode_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(naive_mode_end - fast_mode_end).count());

			DS::free_tree(tree);
		}
		auto avg_gen = ((long)(std::accumulate(generation_times.begin(), generation_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_build = ((long)(std::accumulate(tree_build_times.begin(), tree_build_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_tree_query = ((long)(std::accumulate(tree_query_times.begin(), tree_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_less_naive = ((long)(std::accumulate(less_naive_query_times.begin(), less_naive_query_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_fast_mode = ((long)(std::accumulate(fast_mode_times.begin(), fast_mode_times.end(), 0) / num_runs / 100.0)) / 10.0;
		auto avg_naive_mode = ((long)(std::accumulate(naive_mode_times.begin(), naive_mode_times.end(), 0) / num_runs / 100.0)) / 10.0;

		//std::cout << std::setprecision(1);
		std::cout << avg_gen << " & " << avg_build << " & " << avg_tree_query << " & " << avg_less_naive << " & " << avg_fast_mode << " & " << avg_naive_mode << "\\\\" << std::endl;

		std::ofstream gen_raw("..\\results\\gen.txt", std::ios_base::app);
		std::ofstream build_raw("..\\results\\build.txt", std::ios_base::app);
		std::ofstream tree_range_raw("..\\results\\tree_range.txt", std::ios_base::app);
		std::ofstream naive_range_raw("..\\results\\naive_range.txt", std::ios_base::app);
		std::ofstream fast_mode_raw("..\\results\\fast_mode.txt", std::ios_base::app);
		std::ofstream naive_mode_raw("..\\results\\naive_mode.txt", std::ios_base::app);

		for (int i = 0; i < num_runs; i++) {
			gen_raw << generation_times[i] << (i != num_runs - 1 ? ", " : "");
			build_raw << tree_build_times[i] << (i != num_runs - 1 ? ", " : "");
			tree_range_raw << tree_query_times[i] << (i != num_runs - 1 ? ", " : "");
			naive_range_raw << less_naive_query_times[i] << (i != num_runs - 1 ? ", " : "");
			fast_mode_raw << fast_mode_times[i] << (i != num_runs - 1 ? ", " : "");
			naive_mode_raw << naive_mode_times[i] << (i != num_runs - 1 ? ", " : "");
		}

		gen_raw << std::endl;
		build_raw << std::endl;
		tree_range_raw << std::endl;
		naive_range_raw << std::endl;
		fast_mode_raw << std::endl;
		naive_mode_raw << std::endl;

		gen_raw.close();
		build_raw.close();
		tree_range_raw.close();
		naive_range_raw.close();
		fast_mode_raw.close();
		naive_mode_raw.close();
	}

	static void run_1d_single(
		int num_runs, 
		int num_queries, 
		int k, 
		int num_items, 
		double min, 
		double max, 
		unsigned int Delta, 
		double gamma, 
		double alpha
	) {
		std::vector<std::vector<Point>> points = {};
		for (int i = 0; i < num_runs; i++) {
			points.push_back(generate_sequence(min, max, num_items, Delta, gamma, alpha));
		}

		run_1d_single(num_queries, k, &points);
	}

	static void run_1d_generated(int start = 0, int end = 12, std::string name = "A") {
		int num_runs = 10;
		int Q = 1000;
		// n, Delta, gamma, alpha

		// A1-A12
		double scenarios[12][4] = {
			{ 1000, 20, 0, 0 },
			{ 10000, 20, 0, 0 },
			{ 100000, 20, 0, 0 },
			{ 1000, 100, 0, 0 },
			{ 10000, 100, 0, 0 },
			{ 100000, 100, 0, 0 },
			{ 1000, 20, 30, 0.95 },
			{ 10000, 20, 30, 0.95 },
			{ 100000, 20, 30, 0.95 },
			{ 1000, 100, 150, 0.95 },
			{ 10000, 100, 150, 0.95 },
			{ 100000, 100, 150, 0.95 },
		};
		//B1-B2
		/*double scenarios[2][4] = {
			{ 10000, 100, 0, 0 },
			{ 10000, 100, 150, 0.95 },
		}*/;

		// A1-A12
		double ks[] = { 10, 25, 50, 75, 100, 250, 500, 750, 1000 };
		//double ks[] = { 1500, 2000 };
		// B1, B2
		//double ks[] = { 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000 };

		std::cout << "Scenario & gen & build & tree range & naive range & fast mode & naive mode \\\\" << std::endl;
		
		for (int i = start; i < end; i++) {
			if (scenarios[i][0] != 10000) continue;
			for (int k = 0; k < (scenarios[i][0] > 1000 ? 9 : 8); k++) {
		/*for (int i = 0; i < 2; i++) {
			for (int k = 0; k < 9; k++) {*/
				std::cout << name << i + 1 << ", k=" << ks[k] << " & ";
				run_1d_single(
					num_runs,
					Q, 
					ks[k], 
					(int)scenarios[i][0], 
					-50000, 
					50000, 
					(unsigned int)scenarios[i][1], 
					scenarios[i][2], 
					scenarios[i][3]
				);
			}
			std::cout << "\\hline" << std::endl;
		}
	}

	static void run_1d_real() {
		int Q = 1000;
		int dim = 1;

		std::vector<std::string> files = {
			"temperature-02-06-2024.points",
			"temperature-03-06-2024.points",
			"temperature-04-06-2024.points",
			"temperature-05-06-2024.points",
			"temperature-06-06-2024.points",
			"temperature-07-06-2024.points",
			"temperature-08-06-2024.points",
			"temperature-09-06-2024.points",
			"temperature-10-06-2024.points",
			"temperature-11-06-2024.points",
		};

		double ks[] = { 1500, 2000 };
		std::cout << "Scenario & gen & build & tree range & naive range & fast mode & naive mode \\\\" << std::endl;

		for (int k = 0; k < 9; k++) {
			std::cout << "C" << dim + 1 << ", k=" << ks[k] << " & ";
			std::vector<std::vector<Point>> points = {};
			for (int i = 0; i < files.size(); i++) {
				points.push_back(read_file("..\\data\\" + files[i], 0));
			}

			run_1d_single(Q, ks[k], &points);
		}
	}
}