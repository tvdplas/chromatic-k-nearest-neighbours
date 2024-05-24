#include <iostream>
#include <random>
#include <vector>

#include "datastructures/1d_mode.cpp"


int main()
{
    int n = 100, Delta = 10;
    auto A = DS::generate_colors(n, Delta); // color generation already does rank reduction
    auto Q = DS::generate_Q(A);
    auto APrime = DS::generate_A_prime(A, Q);
    

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

