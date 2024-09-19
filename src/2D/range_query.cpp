#pragma once
#include <vector>
#include <algorithm>
#include "rangetree.h"
#include <CGAL/Cartesian_d.h>
#include <CGAL/Kernel_d/Point_d.h>

namespace N2D {
	typedef unsigned int uint;
	typedef uint Color;
	typedef RangeTree::RangeTree<Color>* Tree;

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

	static std::vector<NumTy> generate_sorted_dim(std::vector<Point_d>* locations, uint dim) {
		if (dim > 1) 
			throw std::logic_error("Queries with |dims| > 2 are not tested; proceed with caution.");
		if (!locations->size())
			throw std::logic_error("Location list does not contain any entries.");
		if (dim >= (*locations)[0].dimension()) 
			throw std::logic_error("Tried to create sorted set of points on dimension higher than pointset dim");

		std::vector<NumTy> sorted_dim(locations->size() + 2);
		sorted_dim[0] = std::numeric_limits<NumTy>::lowest();
		sorted_dim[locations->size() + 1] = std::numeric_limits<NumTy>::max();

		for (int i = 0; i < locations->size(); i++) {
			sorted_dim[i + 1] = ((*locations)[i][dim]);
		}
		
		std::sort(sorted_dim.begin(), sorted_dim.end());
		return sorted_dim;
	}

	static NumTy get_smallest_distance_lower(Tree tree, std::vector<NumTy>* sorted, Point_d q, int k, uint dim) {
		if (dim > 1)
			throw std::logic_error("Not tested on dim > 2");

		// find index of first item smaller than or equal to query point.x
		int l = 0;
		int u = std::distance(sorted->begin(), std::lower_bound(sorted->begin(), sorted->end(), q[dim]));
		int res = -1;

		while (l <= u) {
			int m = (l + u) / 2;
			NumTy distance = std::abs(q[dim] - (*sorted)[m]);
			Point_d lower({ q.x() - distance, q.y() - distance }), upper({ q.x() + distance, q.y() + distance });

			auto count = tree->countInRange(lower, upper);
			if (count >= k) {
				res = m;
				l = m + 1;
			}
			else {
				u = m - 1;
			}
		}

		if (res == -1) {
			// for during debug with k < N/2
			// throw std::logic_error("Binary search for smallest k-square failed");
			// for during actual runs as k > N/2 can result in one of the two halfspaces not containing enough points 
			return std::numeric_limits<NumTy>::max();
		}

		return  std::abs(q[dim] - (*sorted)[res]);
	}

	static NumTy get_smallest_distance_upper(Tree tree, std::vector<NumTy>* sorted, Point_d q, int k, uint dim) {
		if (dim > 1)
			throw std::logic_error("Not tested on dim > 2");

		// find index of first item smaller than or equal to query point.x
		int l = std::distance(sorted->begin(), std::upper_bound(sorted->begin(), sorted->end(), q[dim]));
		int u = sorted->size() - 1;
		int res = -1;

		while (l <= u) {
			int m = (l + u) / 2;
			NumTy distance = std::abs(q[dim] - (*sorted)[m]);
			Point_d lower({ q.x() - distance, q.y() - distance }), upper({ q.x() + distance, q.y() + distance });

			auto count = tree->countInRange(lower, upper);
			if (count >= k) {
				res = m;
				u = m - 1;
			}
			else {
				l = m + 1;
			}
		}

		if (res == -1) {
			// for during debug with k < N/2
			// throw std::logic_error("Binary search for smallest k-square failed");
			// for during actual runs as k > N/2 can result in one of the two halfspaces not containing enough points 
			return std::numeric_limits<NumTy>::max();
		}

		return  std::abs(q[dim] - (*sorted)[res]);
	}

	// Returns radius from q
	static NumTy query_k_nearest(Tree tree, std::vector<NumTy>* sorted_x, std::vector<NumTy>* sorted_y, Point_d q, int k) {
		NumTy x_lower = get_smallest_distance_lower(tree, sorted_x, q, k, 0);
		NumTy y_lower = get_smallest_distance_lower(tree, sorted_y, q, k, 1);
		NumTy x_upper = get_smallest_distance_upper(tree, sorted_x, q, k, 0);
		NumTy y_upper = get_smallest_distance_upper(tree, sorted_y, q, k, 1);

		return std::min({ x_lower, y_lower, x_upper, y_upper });
	}
}