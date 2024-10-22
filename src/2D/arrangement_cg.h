#pragma once
#include <CGAL/Random.h>
#include <CGAL/draw_arrangement_2.h>

struct Default_color_generator {
    /*! Obtain color
     */
    template <typename HalfedgeHandle>
    CGAL::IO::Color operator()(HalfedgeHandle /* h */) {
        static std::random_device rd;
        static std::mt19937 rng(rd());
        static std::uniform_int_distribution<int> uni(0, 255);
        return CGAL::IO::Color(uni(rng), uni(rng), uni(rng));
    }
};