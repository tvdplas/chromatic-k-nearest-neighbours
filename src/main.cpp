#include <iostream>
#include <random>
#include <array> 
#include <CGAL/Simple_cartesian.h>
#include "datastructures/tree.cpp";

#define DEBUG_PRINTING

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

void get_random_points(Point_2 points[], int point_count) {
    const int min_x = -50;
    const int max_x = 50;
    std::uniform_real_distribution<double> rnd(min_x, max_x);
    std::default_random_engine re;

    for (int i = 0; i < point_count; i++) {
        // fix y coordinate at 0 as there is no Point_1 class. 
        points[i] = Point_2(rnd(re), 0);
        #ifdef DEBUG_PRINTING
        std::cout << "p" << i << ": " << points[i] << std::endl;
        #endif
    }
}

int main()
{
    const int point_count = 10;
    Point_2 points[point_count];
    get_random_points(points, point_count);

    int list[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto tree = DS::generate_tree<10, int>(list);
    DS::cleanup_tree(tree);


    
    std::string str;
    std::cin >> str;
    return 0;
}

