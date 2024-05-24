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
    auto S_S_Prime = DS::generate_S_S_Prime(A, (uint)std::sqrt(A.size()));
    
    auto naive_mode = DS::get_mode_naive(A, 14, 93);
    auto mode = DS::get_mode(A, APrime, Q, S_S_Prime.S, S_S_Prime.SPrime, 14, 93, (uint)std::sqrt(A.size()));

    

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}

