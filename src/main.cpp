#include <iostream>
#include <random>
#include <vector>
#include "tests/1d.cpp"


int main()
{
    Test::run_1d(1, 10000, 1000, -25000, 25000, 100, 1000);
#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

