cmake_minimum_required(VERSION 3.28)
project(ctest-demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(core STATIC src/twice.cpp)
target_include_directories(core PUBLIC include)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE core)

enable_testing()

add_executable(tests_sum tests/test_sum.cpp)
target_link_libraries(tests_sum PRIVATE core)
add_test(NAME sum COMMAND tests_sum)
set_tests_properties(sum PROPERTIES TIMEOUT 10 LABELS "unit")

add_executable(tests_twice tests/test_twice.cpp)
target_link_libraries(tests_twice PRIVATE core)
add_test(
  NAME twice
  COMMAND tests_twice
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(twice PROPERTIES TIMEOUT 10 LABELS "unit;smoke"
                                      ENVIRONMENT "CTEST_DEMO_LANG=C++")
