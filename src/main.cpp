#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <CGAL/Simple_cartesian.h>
#include "datastructures/tree.cpp";

#define DEBUG_PRINTING
#define DEBUG_EXCEPTIONS

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

bool less_than(double a, double b) { return a < b; }
double between(double a, double b) { return a / 2 + b / 2; }
double distance(double a, double b) { return abs(a - b); }

int main()
{
    // First, create tree
    std::vector<double> points = { 1, 4, 7, 8, 10, 12, 13, 18 };
    DS::Tree<double>* tree = DS::generate_tree<double>(points, &between);
#ifdef DEBUG_PRINTING
    DS::print_tree(tree);
#endif // DEBUG_PRINTING

    // Then, do query for q = 5 with k = 3
    double q = 0.9, k = 5;
    auto search_path = DS::query_tree<double>(tree, q, &less_than);
    DS::set_tree_counts<double>(search_path);
    auto res = DS::find_k_nearest<double>(tree, k, q, &distance);
    std::cout << res << std::endl;

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

