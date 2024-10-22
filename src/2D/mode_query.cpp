#pragma once
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_dcel_base.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/basic.h>
#include <CGAL/Arr_batched_point_location.h>
#include <CGAL/Point_2.h>
#include <CGAL/Line_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/draw_arrangement_2.h>

using namespace std;

template<class T>
using vec = vector<T>;
typedef vec<set<int>> ConflictList;
typedef struct { 
	int index;
	pair<int, int> mode; 
	map<int, int> under_frequencies;
} FaceData;
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Arr_non_caching_segment_traits_2<Kernel> Traits;
using Dcel = CGAL::Arr_extended_dcel<Traits, bool, bool, FaceData>;
typedef CGAL::Arrangement_2<Traits, Dcel> Arrangement;
typedef Arrangement::Face_handle Face_handle;
typedef Arrangement::Face_const_handle Face_const_handle;
typedef Arrangement::Halfedge_handle Halfedge_handle;
typedef Arrangement::Vertex_handle Vertex_handle;
typedef CGAL::Point_2<Kernel> Point_2;
typedef CGAL::Line_2<Kernel> Line_2;
typedef CGAL::Segment_2<Kernel> Segment_2;
using Point_location_result = CGAL::Arr_point_location_result<Arrangement>;
using Query_result = std::pair<Point_2, Point_location_result::Type>;


template<class T>
static pair<vec<T>, vec<T>> random_split(vec<T>* input, int left_count) {
	std::random_device rd;
	std::mt19937 g(rd());

	// copy, then randomly shuffle array, then slice
	vec<T> copy(input->begin(), input->end());
	shuffle(copy.begin(), copy.end(), g);
	vec<T> left(copy.begin(), copy.begin() + left_count);
	vec<T> right(copy.begin() + left_count, copy.end());
	return pair<vec<T>, vec<T>>(left, right);
}

static vec<Line_2> get_dual_lines(vec<Point_2>* locations) {
	vec<Line_2> dual_lines(locations->size());
	for (int i = 0; i < locations->size(); i++) {
		auto loc = (*locations)[i];
		dual_lines[i] = Line_2(loc.y(), loc.x(), -1);
	}
	return dual_lines;
}

// Returns a set of line segments that have been clamped to the bounding box provided by lower and upper
static vec<Segment_2> get_segments(vec<Line_2>* lines, Point_2 lower_left, Point_2 upper_right, bool include_boundaries) {
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
				if (const Point_2* p = get_if<Point_2>(&*result)) {
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

static void triangulate_arrangement(Arrangement* arr) {
	for (Face_handle face = arr->faces_begin(); face != arr->faces_end(); face++) {
		if (face->is_unbounded()) continue;

		Halfedge_handle start = *face->outer_ccbs_begin(); // can be directly dereferenced as we only have 1
		Halfedge_handle curr = start;
		vec<Halfedge_handle> halfedges;

		// Get edge list
		do {
			halfedges.push_back(curr);
			curr = curr->next();
		} while (curr != start);

		if (halfedges.size() <= 3) continue; // already triangulated

		// Triangulate by connecting each vertex except neighbours to the first.
		auto p1 = halfedges[0]->target()->point();
		for (int i = 2; i < halfedges.size() - 1; i++) {
			Segment_2 edge(Segment_2(p1, halfedges[i]->target()->point()));
			arr->insert_at_vertices(edge, halfedges[0], halfedges[i]);
		}
	}
}

// Returns a set of intersecting lines per face.
static ConflictList get_conflict_list(Arrangement* arr, vec<Segment_2>* rest_segments) {
	// Per face, maintain a set of lines that intersect it. 
	int F = arr->number_of_faces();
	ConflictList conflict_list(F);

	int face_index= 0;
	for (Face_handle face = arr->faces_begin(); face != arr->faces_end(); face++, face_index++) {
		if (face->is_unbounded()) continue;

		// Set index of face for future reference
		FaceData fd = {
			face_index
		};
		face->set_data(fd);

		conflict_list[face_index] = set<int>();
		
		// Test intersection with the segments describing the face
		Halfedge_handle start = *face->outer_ccbs_begin();
		Halfedge_handle curr = start;
		do {
			Segment_2 edge(curr->source()->point(), curr->target()->point());

			// Iterate over all segments
			for (int segment_index = 0; segment_index < rest_segments->size(); segment_index++) {
				auto line = (*rest_segments)[segment_index];
				auto result = CGAL::intersection(line, edge);

				if (result && get_if<Point_2>(&*result)) {
					// We dont actually care about where the intersection is, 
					// just that it happened
					conflict_list[face_index].insert(segment_index);
				}
			}		
			curr = curr->next();
		} while (curr != start);
	}

	return conflict_list;
}

static bool is_valid_r_cutting(ConflictList* cl, int N, int r) {
	int max = (int)round((double)N / (double)r);
	// Check if any face has more than r intersecting lines
	for (auto s = cl->begin(); s < cl->end(); s++) {
		if (s->size() > max) return false;
	}
	return true;
}

static void annotate_arrangement(Arrangement* arr, ConflictList* cl, vec<Segment_2>* lines, Point_2 lower, vec<int>* colors) {
	// for each face, annotate with the color that passes through it most
	// along with the frequency of colors below it that also pass through the face
	for (Face_handle face = arr->faces_begin(); face != arr->faces_end(); face++) {
		int face_index = face->data().index;
		vec<set<int>> lines_under_vertices = vec<set<int>>(3);
		for (int i = 0; i < 3; i++) lines_under_vertices[i] = set<int>();
		
		// Find the lines that lie fully below each face
		int edge_index = 0;
		Halfedge_handle curr = *face->outer_ccbs_begin();
		for (int edge_index = 0; edge_index < 3; edge_index++) {
			// Lines that are below the target vertex of the edge
			Point_2 target = curr->target()->point();
			Segment_2 ray_down(target, Point_2(target.x(), lower.y()));
			for (int line_index = 0; line_index < lines->size(); line_index++) {
				auto line = (*lines)[line_index];
				auto res = CGAL::intersection(ray_down, line);
				if (res && get_if<Point_2>(&*res)) {
					// If ray from vertex hits line, add it to that vertex's conflict list
					lines_under_vertices[edge_index].insert(line_index);
				}
			}

			// Go to next halfedge
			curr = curr->next();
		}

		// Once we have the intersection list for each of the vertex rays, 
		// take the intersection in order to get the lines that are fully beneath it.
		set<int> intersect_01;
		set<int> lines_under_face;
		set_intersection(
			lines_under_vertices[0].begin(), lines_under_vertices[0].end(),
			lines_under_vertices[1].begin(), lines_under_vertices[1].end(),
			inserter(intersect_01, intersect_01.begin())
		);
		set_intersection(
			intersect_01.begin(), intersect_01.end(),
			lines_under_vertices[2].begin(), lines_under_vertices[2].end(),
			inserter(lines_under_face, lines_under_face.begin())
		);
		
		// Now, determine color frequency for intersecting lines / lines below
		map<int, int> under_color_counts, intersecting_color_counts;
		for (int line_index : (*cl)[face_index] ) {
			int color = (*colors)[line_index];
			if (intersecting_color_counts.count(color)) intersecting_color_counts[color]++;
			else intersecting_color_counts[color] = 1;
		}
		// Only add color below if it is also present in the intersection list,
		// otherwise we dont need it
		for (int line_index : lines_under_face) {
			int color = (*colors)[line_index];
			if (!intersecting_color_counts[color]) continue;
			if (under_color_counts.count(color)) under_color_counts[color]++;
			else under_color_counts[color] = 1;
		}
		
		// Determine the mode color of intersecting lines
		pair<int, int> mode = *max_element(
			begin(intersecting_color_counts),
			end(intersecting_color_counts),
			[](const pair<int, int> a, const pair<int, int> b) { return a.second < b.second; }
		);

		// Annotate face 
		FaceData fd = {
			face_index,
			mode,
			under_color_counts,
		};
		face->set_data(fd);
	}
}

// Returns the mode color for a certain query point
static pair<int, int> query_arrangement(Arrangement* arr, ConflictList* cl, vec<Segment_2>* lines, vec<int>* colors, Point_2 lower, Point_2 q, double r) {
	// first, find the face that the query point is in.
	vec<Point_2> query_point = { q };
	vec<Query_result> face_result = {};
	CGAL::locate(*arr, query_point.begin(), query_point.end(), back_inserter(face_result));
	if (const Face_const_handle* face_ptr = get_if<Face_const_handle>(&face_result[0].second)) {
		Face_const_handle face = *face_ptr;
		// then, for this face, find which of the lines in the conflict list are below the query point
		
		map<int, int> candidate_modes(face->data().under_frequencies);
		int face_index = face->data().index;
		for (auto line_index : (*cl)[face_index]) {
			// add count if the line is below q
			Segment_2 ray_down(q, Point_2(q.x(), lower.y()));
			auto result = CGAL::intersection(ray_down, (*lines)[line_index]);
			if (result && get_if<Point_2>(&*result)) {
				// Add 1 to the line
				int color = (*colors)[line_index];
				if (candidate_modes.count(color)) candidate_modes[color]++;
				else candidate_modes[color] = 1;
			}
		}

		// Get the maximum candidate mode and returnpair<int, int> mode = *max_element(
		pair<int, int> mode = *max_element(
			begin(candidate_modes),
			end(candidate_modes),
			[](const pair<int, int> a, const pair<int, int> b) { return a.second < b.second; }
		);
		return mode;
	}
	else {
		cout << "Could not locate point " << q << " due to point location not returning a face.";
		return pair<int, int>(-1, 0);
	}
}

static void show_arrangement(Arrangement* arr) {
	
	CGAL::draw(*arr);
}