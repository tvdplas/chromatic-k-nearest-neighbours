cmake_minimum_required(VERSION 3.1)
project( chromatic_k_nearest_neighbours )

set(EXECUTABLE_OUTPUT_PATH "bin")
set(CMAKE_CXX_STANDARD 11)
find_package(CGAL REQUIRED OPTIONAL_COMPONENTS Qt5)

add_executable(chromatic_k_nearest_neighbours
    src/tests/1d.cpp
    src/tests/2d.cpp
    src/1D/range_query.cpp
    src/1D/mode_query.cpp
    src/2D/rangetree.h
    src/2D/range_query.cpp
    src/main.cpp
)

target_include_directories(chromatic_k_nearest_neighbours
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/datastructures
        ${PROJECT_SOURCE_DIR}/src/tests)

if(CGAL_Qt5_FOUND)
  target_link_libraries(chromatic_k_nearest_neighbours PUBLIC CGAL::CGAL_Basic_viewer)
else()
  message(STATUS "NOTICE: Several examples require Qt5 and will not be compiled.")
endif()
