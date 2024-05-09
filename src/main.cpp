#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "datastructures/tree.cpp";

bool leq(double a, double b) { return a <= b; }
double distance(double a, double b) { return abs(a - b); }

int main()
{
    std::vector<double> input = { 1, 2, 3, 4, 5, 6, 7, 8 };
    auto tree = DS::generate_tree(input);
    auto search_path = DS::get_search_path<double>(tree, 4.1, &leq);
    DS::split_tree(search_path);
    auto res = DS::get_k_nearest<double>(tree, 4.1, 4, &distance);

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

