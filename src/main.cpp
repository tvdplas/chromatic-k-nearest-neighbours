#pragma once
#include <iostream>
#include <random>
#include <vector>
#include "tests/2d.cpp"

int main() {
    N2D::run_2d_generated();

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}


