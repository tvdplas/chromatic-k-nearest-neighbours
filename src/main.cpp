#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/draw_polyhedron.h>
#include <fstream>

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef CGAL::Polyhedron_3<Kernel>                       Polyhedron;

int main() {
    Polyhedron P;
    std::ifstream in1("../data/meshes/cross_quad.off");
    in1 >> P;
    CGAL::draw(P);

#pragma region cleanup
    std::string str;
    std::cin >> str;
    return 0;
#pragma endregion
}


