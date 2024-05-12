cmake_minimum_required(VERSION 3.1)
project( chromatic_k_nearest_neighbours )

find_package(CGAL REQUIRED)

set(EXECUTABLE_OUTPUT_PATH "bin")

add_executable(chromatic_k_nearest_neighbours
    src/tests/1d_range_finding.cpp
    src/datastructures/tree.cpp
    src/main.cpp
)

target_include_directories(chromatic_k_nearest_neighbours
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/datastructures
        ${PROJECT_SOURCE_DIR}/src/tests)

target_link_libraries(chromatic_k_nearest_neighbours CGAL::CGAL)