#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "datastructures/tree.cpp";

bool leq(double a, double b) { return a <= b; }
double distance(double a, double b) { return abs(a - b); }
double plusEps(double a, double b) { return std::min(a / 2 + b / 2, a + 0.001); }

int main()
{
    std::vector<double> input = { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto tree = DS::generate_tree<double>(input, &plusEps);
    auto search_path = DS::get_search_path<double>(tree, 4.2, &leq);
    DS::split_tree(search_path);
    auto res = DS::get_k_nearest<double>(tree, 4.2, 2, &distance);
    std::cout << res;

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

