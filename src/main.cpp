#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "datastructures/tree.cpp";

using namespace std;

static bool leq(double a, double b) {
    return a <= b; 
}
static double distance(double a, double b) {
    return abs(a - b);
}

int main()
{
    //std::vector<double> items = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::vector<double> items = { 1, 2, 3, 4, 5, 6, 7 };
    int k = 6;
    double q = 5.1;
    auto tree = DS::generate_tree<double>(items);
    auto path = DS::get_search_path<double>(tree, q, &leq);
    auto split_trees = DS::split_tree(path);
    auto res = DS::get_k_nearest(split_trees[0], split_trees[1], k, q, &distance);

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

