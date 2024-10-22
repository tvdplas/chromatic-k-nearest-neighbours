#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include "tests/2d.cpp"
#include "2D/mode_query.cpp"

int main() {
    int r = 6;
    int N = 100;
    vec<Point_2> points = N2D::generate_locations_mode(-100, 100, N);
    auto lines = get_dual_lines(&points);
    auto split_lines = random_split(&lines, r * log(r)/log(2)); // moet r log r worden
    auto arrangement_segments = get_segments(&split_lines.first, Point_2(-1, -1), Point_2(1, 1), true);
    auto remaining_segments = get_segments(&split_lines.second, Point_2(-1, -1), Point_2(1, 1), false);
    auto arr = create_arrangement(&arrangement_segments);
    triangulate_arrangement(&arr);

    cout << "Number of faces: " << arr.number_of_faces() << endl;
    auto conflict_list = get_conflict_list(&arr , &remaining_segments);
    bool is_cutting = is_valid_r_cutting(&conflict_list, N, r);
    cout << "Is 1/r-cutting? " << (is_cutting ? "Yes" : "No") << endl;
    // Check conflict 

    
//#pragma region cleanup
//    std::string str;
//    std::cin >> str;
//    return 0;
//#pragma endregion
}


