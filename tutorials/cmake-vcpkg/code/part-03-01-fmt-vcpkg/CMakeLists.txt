cmake_minimum_required(VERSION 3.28)
project(fmt-demo LANGUAGES CXX)

find_package(fmt CONFIG REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE fmt::fmt)

if(MSVC)
    target_compile_options(app PRIVATE /utf-8)
endif()
