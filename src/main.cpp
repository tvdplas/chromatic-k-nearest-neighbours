#include <iostream>
#include <random>
#include <vector>
#include "tests/1d.cpp"
#include "datastructures/tree.cpp";


int main()
{
    int start = 0, end = 12;
    std::cin >> start;
    std::cin >> end;
    Test::run_1d_generated(start, end);
    //Test::run_1d_real();
    //auto points = Test::generate_sequence(-50, 50, 100, 10, 5, 0.8);
    //Test::run_1d(10, 10000, 1000, -50000, 50000, 100, 1000);
#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}


