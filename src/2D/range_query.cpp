#pragma once
#include <vector>
#include <algorithm>
#include <chrono> 
#include "rangetree.h"
#include <CGAL/Cartesian_d.h>
#include <CGAL/Kernel_d/Point_d.h>

namespace N2D {
	typedef unsigned int uint;
	typedef int Color;
	typedef RangeTree::RangeTree<Color>* Tree;
	typedef std::pair<NumTy, int> pair_ni;

	static RangeTree::RangeTree<Color> generate_tree(std::vector<Point_d>* locations, std::vector<Color>* colors) {
		if (locations->size() != colors->size())
			throw std::logic_error("Cannot create range tree on list of locations with different length than list of colors");

		int count = locations->size();

		std::vector<RangeTree::RTPoint<Color>> points = { };
		for (int i = 0; i < count; i++) {
			points.push_back(RangeTree::RTPoint<Color>((*locations)[i], (*colors)[i]));
		}

		return RangeTree::RangeTree<Color>(points);
	}

	static std::vector<NumTy> get_sorted_dim_values(std::vector<pair_ni>* sorted_dim_pairs) {
		std::vector<NumTy> sorted_dim(sorted_dim_pairs->size());
		for (int i = 0; i < sorted_dim_pairs->size(); i++) {
			sorted_dim[i] = (*sorted_dim_pairs)[i].first;
		}
		return sorted_dim;
	}

	static std::vector<pair_ni> generate_sorted_dim_pairs(std::vector<Point_d>* locations, uint dim) {
		if (dim > 1)
			throw std::logic_error("Queries with |dims| > 2 are not tested; proceed with caution.");
		if (!locations->size())
			throw std::logic_error("Location list does not contain any entries.");
		if (dim >= (*locations)[0].dimension())
			throw std::logic_error("Tried to create sorted set of points on dimension higher than pointset dim");

		std::vector<pair_ni> sorted_dim_pairs(locations->size() + 2);
		sorted_dim_pairs[0] = pair_ni(std::numeric_limits<NumTy>::lowest(), -1);
		sorted_dim_pairs[locations->size() + 1] = pair_ni(std::numeric_limits<NumTy>::max(), locations->size());

		for (int i = 0; i < locations->size(); i++) {
			sorted_dim_pairs[i + 1] = pair_ni((*locations)[i][dim], i);
		}

		auto comp = [](pair_ni a, pair_ni b) {
			return a.first < b.first;
		};

		std::sort(sorted_dim_pairs.begin(), sorted_dim_pairs.end(), comp);
		return sorted_dim_pairs;
	}

	static std::pair<NumTy, double> get_smallest_distance_lower(Tree tree, std::vector<NumTy>* sorted, Point_d q, int k, uint dim) {
		if (dim > 1)
			throw std::logic_error("Not tested on dim > 2");

		// find index of first item smaller than or equal to query point.x
		int l = 0;
		int u = std::distance(sorted->begin(), std::lower_bound(sorted->begin(), sorted->end(), q[dim]));
		int res = -1;
		int rt_bin_searches = 0;
		std::vector<long> bin_search_iteration_time = {};

		while (l <= u) {
			auto start = std::chrono::high_resolution_clock::now();

			int m = (l + u) / 2;
			NumTy distance = std::abs(q[dim] - (*sorted)[m]);

			Point_d lower({ q.x() - distance, q.y() - distance }), upper({ q.x() + distance, q.y() + distance });

			auto query_res = tree->countInRange(lower, upper);
			auto count = query_res.first;
			rt_bin_searches++;

			if (count >= k) {
				res = m;
				l = m + 1;
			}
			else {
				u = m - 1;
			}

			auto end = std::chrono::high_resolution_clock::now();
			bin_search_iteration_time.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		}

		auto avg_time_per_iteration = ((double)(std::accumulate(
			bin_search_iteration_time.begin(),
			bin_search_iteration_time.end(),
			0.0) / rt_bin_searches));

		if (res == -1) {
			return std::pair<NumTy, double>(std::numeric_limits<NumTy>::max(), avg_time_per_iteration);
		}

		return std::pair<NumTy, double>(std::abs(q[dim] - (*sorted)[res]), avg_time_per_iteration);
	}

	static std::pair<NumTy, double> get_smallest_distance_upper(Tree tree, std::vector<NumTy>* sorted, Point_d q, int k, uint dim) {
		if (dim > 1)
			throw std::logic_error("Not tested on dim > 2");

		// find index of first item smaller than or equal to query point.x
		int l = std::distance(sorted->begin(), std::upper_bound(sorted->begin(), sorted->end(), q[dim]));
		int u = sorted->size() - 1;
		int res = -1;
		int rt_bin_searches = 0;

		std::vector<long> bin_search_iteration_time = {};

		while (l <= u) {
			auto start = std::chrono::high_resolution_clock::now();

			int m = (l + u) / 2;
			NumTy distance = std::abs(q[dim] - (*sorted)[m]);

			Point_d lower({ q.x() - distance, q.y() - distance }), upper({ q.x() + distance, q.y() + distance });
			auto query_res = tree->countInRange(lower, upper);
			auto count = query_res.first;
			rt_bin_searches++;
			if (count >= k) {
				res = m;
				u = m - 1;
			}
			else {
				l = m + 1;
			}

			auto end = std::chrono::high_resolution_clock::now();
			bin_search_iteration_time.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		}

		auto avg_time_per_iteration = ((double)(std::accumulate(
			bin_search_iteration_time.begin(),
			bin_search_iteration_time.end(),
			0.0) / rt_bin_searches)); // us

		if (res == -1) {
			return std::pair<NumTy, double>(std::numeric_limits<NumTy>::max(), avg_time_per_iteration);
		}

		return std::pair<NumTy, double>(std::abs(q[dim] - (*sorted)[res]), avg_time_per_iteration);
	}

	// Returns radius from q
	static std::pair<NumTy, double> query_k_nearest(Tree tree, std::vector<NumTy>* sorted_x, std::vector<NumTy>* sorted_y, Point_d q, int k) {
		auto x_lower = get_smallest_distance_lower(tree, sorted_x, q, k, 0);
		auto y_lower = get_smallest_distance_lower(tree, sorted_y, q, k, 1);
		auto x_upper = get_smallest_distance_upper(tree, sorted_x, q, k, 0);
		auto y_upper = get_smallest_distance_upper(tree, sorted_y, q, k, 1);

		return std::pair<NumTy, int>(
			std::min({ x_lower.first, y_lower.first, x_upper.first, y_upper.first }),
			x_lower.second + y_lower.second + x_upper.second + y_upper.second
		);
	}
}