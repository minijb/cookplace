cmake_minimum_required(VERSION 3.16)

project(cpp17_type_conversion_tutorial LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(tc_01_overview 01_overview.cpp)
add_executable(tc_02_implicit 02_implicit.cpp)
add_executable(tc_03_user_defined 03_user_defined.cpp)
add_executable(tc_04_named_casts 04_named_casts.cpp)
add_executable(tc_05_dynamic_cast 05_dynamic_cast.cpp)
add_executable(tc_06_type_punning 06_type_punning.cpp)
add_executable(tc_07_ue_cast_sim 07_ue_cast_sim.cpp)
add_executable(tc_08_exercise 08_exercise.cpp)

add_test(NAME tc_01_overview COMMAND tc_01_overview)
add_test(NAME tc_02_implicit COMMAND tc_02_implicit)
add_test(NAME tc_03_user_defined COMMAND tc_03_user_defined)
add_test(NAME tc_04_named_casts COMMAND tc_04_named_casts)
add_test(NAME tc_05_dynamic_cast COMMAND tc_05_dynamic_cast)
add_test(NAME tc_06_type_punning COMMAND tc_06_type_punning)
add_test(NAME tc_07_ue_cast_sim COMMAND tc_07_ue_cast_sim)
add_test(NAME tc_08_exercise COMMAND tc_08_exercise)
