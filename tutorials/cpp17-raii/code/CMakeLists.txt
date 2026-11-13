cmake_minimum_required(VERSION 3.16)

project(cpp17_raii_tutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(raii_01_lifetime 01_lifetime.cpp)
add_executable(raii_02_construction 02_construction.cpp)
add_executable(raii_03_destructors 03_destructors.cpp)
add_executable(raii_04_copy_move 04_copy_move.cpp)
add_executable(raii_05_exception_safety 05_exception_safety.cpp)
add_executable(raii_06_game_resources 06_game_resources.cpp)

add_test(NAME raii_01_lifetime COMMAND raii_01_lifetime)
add_test(NAME raii_02_construction COMMAND raii_02_construction)
add_test(NAME raii_03_destructors COMMAND raii_03_destructors)
add_test(NAME raii_04_copy_move COMMAND raii_04_copy_move)
add_test(NAME raii_05_exception_safety COMMAND raii_05_exception_safety)
add_test(NAME raii_06_game_resources COMMAND raii_06_game_resources)
