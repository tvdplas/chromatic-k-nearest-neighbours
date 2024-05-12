#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "datastructures/tree.cpp";
#include "tests/1d_range_finding.cpp";

#define DEBUG_PRINT

using namespace std;

static bool leq(double a, double b) {
    return a <= b; 
}
static double distance(double a, double b) {
    return abs(a - b);
}

int main()
{
    Test::run_1d_range_finding(1, 10000, 1000, -50000, 50000, 100);


#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

