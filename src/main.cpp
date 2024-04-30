#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <CGAL/Simple_cartesian.h>
#include "datastructures/tree.cpp";

#define DEBUG_PRINTING
#define DEBUG_EXCEPTIONS

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_2 Point_2;
typedef Kernel::Segment_2 Segment_2;

void get_random_points(std::vector<Point_2>* points, int point_count) {
    const int min_x = -1000;
    const int max_x = 1000;
    std::uniform_real_distribution<double> rnd(min_x, max_x);
    std::default_random_engine re;

    for (int i = 0; i < point_count; i++) {
        // fix y coordinate at 0 as there is no Point_1 class. 
        points->push_back(Point_2(rnd(re), 0));
        #ifdef DEBUG_PRINTING
        std::cout << "p" << i << ": " << (*points)[i] << std::endl;
        #endif
    }
}

bool less_than(Point_2 p1, Point_2 p2) {
    return p1.x() < p2.x();
}

int main()
{
    const int point_count = 10;
    std::vector<Point_2> points;
    get_random_points(&points, point_count);
    std::sort(points.begin(), points.end(), &less_than);

    auto tree = DS::generate_tree<Point_2>(points, [](Point_2 p1, Point_2 p2) { return Point_2(p1.x() / 2 + p2.x() / 2, 0);  });
    DS::print_tree<Point_2>(tree);
    std::vector<DS::Tree<Point_2>*> search_path;
    auto res = DS::query_tree<Point_2>(tree, Point_2(100, 0), &less_than, &search_path);
    std::vector<int> count_copies;
    for (auto i = search_path.begin(); i < search_path.end(); i++) {
        (*i)->blocked = DS::Split;
        // search path counts are set to 1 bc of unbalanced tree
        // should be reset later
        count_copies.push_back((*i)->count);
        if (i == search_path.begin()) (*i)->count = (*i)->right->count + 1;
        else (*i)->count = 1;
    }

    // for root_le, switch right and left subtree if needed
    auto root_le = tree, root_geq = tree;
    unsigned int k = 5;

    // blue == le
    while (root_le->type != DS::Leaf && root_geq->type != DS::Leaf) {
        unsigned int geq_count = root_geq->blocked == DS::Left ? 0 : root_geq->left->count;
        unsigned int l = root_le->right->count + geq_count + 1;
        if (l <= k) {
            // update pointer for the blue (le) root to go to the right (actually left)
            root_le = root_le->left;
            // Then, block off the left subtree of the red (geq) root
            root_geq -> blocked = DS::Left;
            // Dit moet even gereset worden later; wss iets met een array aan pointers samen met de rest van de counts;
            root_geq->count = root_geq->count - root_geq->right->count;

            k = k - l;
        }
        else {
            // nu alleen rode tree update :3
            root_geq = root_geq->right;
        }
    }

    DS::cleanup_tree(tree);
    
    std::string str;
    std::cin >> str;
    return 0;
}

