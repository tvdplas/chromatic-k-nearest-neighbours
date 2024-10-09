#pragma once
#include <vector>
#include <set>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/draw_arrangement_2.h>

using namespace std;

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Arr_non_caching_segment_traits_2<Kernel> Traits;
typedef CGAL::Arrangement_2<Traits> Arrangement;
typedef Arrangement::Face_handle Face_handle;
typedef Arrangement::Halfedge_handle Halfedge_handle;
typedef Arrangement::Vertex_handle Vertex_handle;
typedef CGAL::Point_2<Kernel> Point_2;
typedef CGAL::Line_2<Kernel> Line_2;
typedef CGAL::Segment_2<Kernel> Segment_2;
template<class T>
using vec = vector<T>;


static vec<Line_2> get_dual_lines(vec<Point_2>* locations) {
	vec<Line_2> dual_lines(locations->size());
	for (int i = 0; i < locations->size(); i++) {
		auto loc = (*locations)[i];
		dual_lines[i] = Line_2(loc.y(), loc.x(), -1);
	}
	return dual_lines;
}

// Returns a set of line segments that have been clamped to the bounding box provided by lower and upper
static vec<Segment_2> get_segments(vec<Line_2>* lines, Point_2 lower_left, Point_2 upper_right, bool include_boundaries = true) {
	vec<Segment_2> segments = {};
	Segment_2 bottom_edge(lower_left, Point_2(upper_right.x(), lower_left.y()));
	Segment_2 top_edge(Point_2(lower_left.x(), upper_right.y()), upper_right);
	Segment_2 left_edge(lower_left, Point_2(lower_left.x(), upper_right.y()));
	Segment_2 right_edge(Point_2(upper_right.x(), lower_left.y()), upper_right);
	std::vector<Segment_2> box_edges = { bottom_edge, top_edge, left_edge, right_edge };

	for (int i = 0; i < lines->size(); i++) {
		Line_2 line = (*lines)[i];
		Point_2 p1, p2;
		
		std::vector<Point_2> intersection_points;
		for (const Segment_2& edge : box_edges) {
			auto result = CGAL::intersection(line, edge);

			if (result) {
				if (const Point_2* p = boost::get<Point_2>(&*result)) {
					intersection_points.push_back(*p);
				}
			}
		}

		if (intersection_points.size() == 2)
			segments.push_back(Segment_2(intersection_points[0], intersection_points[1]));
	}
	if (include_boundaries) {
		int n = lines->size();
		segments.push_back(bottom_edge);
		segments.push_back(top_edge);
		segments.push_back(left_edge);
		segments.push_back(right_edge);
	}

	return segments;
}


static Arrangement create_arrangement(vec<Segment_2>* segments) {
	Arrangement arr;
	CGAL::insert(arr, segments->begin(), segments->end());
	return arr;
}

static Arrangement triangulate_arrangement(Arrangement arr) {
	for (Face_handle face = arr.faces_begin(); face != arr.faces_end(); ++face) {
		if (face->is_unbounded()) continue;
			
		auto start = *face->outer_ccbs_begin();
		auto curr = start;
		vec<Halfedge_handle> halfedges;
		
		do {
			halfedges.push_back(curr);
			curr = curr->next();
		} while (curr != start);
		
		if (halfedges.size() <= 3) continue; // already triangulated
		
		cout << "Starting triangulation";
		// Triangulate by connecting each vertex except neighbours to the first.
		auto p1 = halfedges[0]->target()->point();
		for (int i = 2; i < halfedges.size() - 1; i++) {
			Segment_2 segment(Segment_2(p1, halfedges[i]->target()->point()));
			arr.insert_at_vertices(segment, halfedges[0], halfedges[i]);
		}
	}

	return arr;
}

static void show_arrangement(Arrangement* arr) {
	CGAL::draw(*arr);
}