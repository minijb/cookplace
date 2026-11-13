#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

#if defined(CH02_HAS_PARALLEL_ALGORITHMS)
#include <execution>
#endif

int main() {
    std::vector<std::uint64_t> values(10000);
    std::iota(values.begin(), values.end(), std::uint64_t{1});
    constexpr std::uint64_t expected = 50005000;

#if defined(CH02_HAS_PARALLEL_ALGORITHMS)
    const auto total = std::reduce(std::execution::par, values.begin(), values.end(),
        std::uint64_t{0});
    std::cout << "parallel execution path\n";
#else
    const auto total = std::accumulate(values.begin(), values.end(), std::uint64_t{0});
    std::cout << "serial fallback: parallel algorithms unavailable in this toolchain\n";
#endif

    assert(total == expected);
    std::cout << "sum=" << total << '\n';
}
