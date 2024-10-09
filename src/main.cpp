#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include "tests/2d.cpp"
#include "2D/mode_query.cpp"

int main() {
    vec<Point_2> points = N2D::generate_locations_mode(-100, 100, 20);
    auto lines = get_dual_lines(&points);
    auto segments = get_segments(&lines, Point_2(-1, -1), Point_2(1, 1), true);
    auto arr = create_arrangement(&segments);
    cout << "Number of faces: " << arr.number_of_faces() << endl;
    arr = triangulate_arrangement(arr);
    show_arrangement(&arr);

    
//#pragma region cleanup
//    std::string str;
//    std::cin >> str;
//    return 0;
//#pragma endregion
}


