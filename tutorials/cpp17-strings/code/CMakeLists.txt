cmake_minimum_required(VERSION 3.16)

project(cpp17_strings_tutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The tutorial examples use assertions as executable correctness checks.
# Keep them enabled in Release without redefining NDEBUG on the command line.
foreach(config RELEASE RELWITHDEBINFO MINSIZEREL)
    string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}}")
    string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_${config} "${CMAKE_CXX_FLAGS_${config}}")
endforeach()

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(strings_01_types 01_types.cpp)
add_executable(strings_02_operations 02_operations.cpp)
add_executable(strings_03_views 03_views.cpp)
add_executable(strings_04_utf8 04_utf8.cpp)
add_executable(strings_05_memory 05_memory.cpp)
add_executable(strings_06_conversion 06_conversion.cpp)
add_executable(strings_07_pmr 07_pmr.cpp)
add_executable(strings_08_benchmark 08_benchmark.cpp)

add_test(NAME strings_01_types COMMAND strings_01_types)
add_test(NAME strings_02_operations COMMAND strings_02_operations)
add_test(NAME strings_03_views COMMAND strings_03_views)
add_test(NAME strings_04_utf8 COMMAND strings_04_utf8)
add_test(NAME strings_05_memory COMMAND strings_05_memory)
add_test(NAME strings_06_conversion COMMAND strings_06_conversion)
add_test(NAME strings_07_pmr COMMAND strings_07_pmr)
add_test(NAME strings_08_benchmark COMMAND strings_08_benchmark)
