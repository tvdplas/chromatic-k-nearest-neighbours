#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "datastructures/tree.cpp";

using namespace std;

static bool leq(double a, double b) {
    return a <= b; 
}

int main()
{
    //std::vector<double> items = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    std::vector<double> items = { 1, 2, 3, 4, 5, 6, 7 };
    auto tree = DS::generate_tree<double>(items);
    auto path = DS::get_search_path<double>(tree, 3.5, &leq);
    auto split_trees = DS::split_tree(path);


#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

