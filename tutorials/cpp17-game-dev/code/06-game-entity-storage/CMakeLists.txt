add_executable(ch02_entity_storage main.cpp)
add_executable(ch02_stl_benchmark benchmark.cpp)
add_executable(ch02_pmr pmr.cpp)
add_executable(ch02_parallel_algorithms parallel.cpp)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("\
#include <execution>\n\
#include <numeric>\n\
#include <vector>\n\
int main() {\n\
    std::vector<int> values{1, 2, 3};\n\
    return std::reduce(std::execution::par, values.begin(), values.end(), 0) == 6 ? 0 : 1;\n\
}\n" CH02_HAS_PARALLEL_ALGORITHMS)

if(CH02_HAS_PARALLEL_ALGORITHMS)
    target_compile_definitions(ch02_parallel_algorithms PRIVATE CH02_HAS_PARALLEL_ALGORITHMS=1)
endif()

add_test(NAME ch02_entity_storage COMMAND ch02_entity_storage)
add_test(NAME ch02_stl_benchmark COMMAND ch02_stl_benchmark)
add_test(NAME ch02_pmr COMMAND ch02_pmr)
add_test(NAME ch02_parallel_algorithms COMMAND ch02_parallel_algorithms)
