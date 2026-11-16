cmake_minimum_required(VERSION 3.21)
project(opengl_part00_ch06 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm CONFIG REQUIRED)
add_executable(ch06_coordinates main.cpp)
target_link_libraries(ch06_coordinates PRIVATE glm::glm)

if(MSVC)
    target_compile_options(ch06_coordinates PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME ch06_coordinate_examples COMMAND ch06_coordinates)
endif()
