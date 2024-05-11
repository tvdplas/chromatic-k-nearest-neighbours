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
    int k = 4;
    double q = 3.2;
    auto tree = DS::generate_tree<double>(items);
    auto res = DS::query_k_nearest<double>(tree, k, q, &leq, &distance);
    auto res2 = DS::query_k_nearest<double>(tree, k + 1, q, &leq, &distance);
    auto res3 = DS::query_k_nearest<double>(tree, k, q - 1, &leq, &distance);
    //auto res = DS::get_k_nearest_single(split_trees[1], 2, DS::Right);

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

