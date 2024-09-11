#include <iostream>
#include <random>
#include <vector>
#include "2D/rangetree.h"
#include <CGAL/Cartesian_d.h>
#include <CGAL/Kernel_d/Point_d.h>

typedef CGAL::Cartesian_d<double>::Point_d Point_d;

namespace RT = RangeTree;
int main() {
    std::vector<Point_d> locations = { Point_d({ 1.0, 1.0 }), Point_d({ 2.0, 2.0}), };
    std::vector<int> colors = { 0, 1 };
    int count = locations.size();

    std::vector<RT::RTPoint<int>> points = { };
    for (int i = 0; i < count; i++) {
        points.push_back(RT::RTPoint<int>(locations[i], colors[i]));
    }

    Point_d lower({ 0.0, 0.0 }), upper({ 3.0, 3.0 });

    RT::RangeTree<int> rtree(points);
    auto res = rtree.countInRange(lower, upper);
    std::cout << res;

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}


