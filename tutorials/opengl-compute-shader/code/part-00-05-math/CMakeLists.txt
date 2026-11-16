cmake_minimum_required(VERSION 3.21)
project(opengl_part00_ch05 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm CONFIG REQUIRED)
add_executable(ch05_math main.cpp)
target_link_libraries(ch05_math PRIVATE glm::glm)

if(MSVC)
    target_compile_options(ch05_math PRIVATE /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME ch05_math_examples COMMAND ch05_math)
endif()
