#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

std::string build_without_reserve(std::string_view name, int score) {
    std::string result;
    result += "player:";
    result += name;
    result += " score:";
    result += std::to_string(score);
    return result;
}

std::string build_with_reserve(std::string_view name, int score) {
    const std::string score_text = std::to_string(score);
    std::string result;
    result.reserve(7 + name.size() + 7 + score_text.size());
    result.append("player:");
    result.append(name.data(), name.size());
    result.append(" score:");
    result.append(score_text);
    return result;
}

template <typename Operation>
double measure_ms(std::size_t iterations, Operation operation) {
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        operation(i);
    }
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

int main() {
    constexpr std::size_t iterations = 20000;
    std::size_t checksum_without = 0;
    std::size_t checksum_with = 0;

    const double without_ms = measure_ms(iterations, [&](std::size_t i) {
        const auto text = build_without_reserve("alice", static_cast<int>(i));
        checksum_without += text.size();
    });

    const double with_ms = measure_ms(iterations, [&](std::size_t i) {
        const auto text = build_with_reserve("alice", static_cast<int>(i));
        checksum_with += text.size();
    });

    if (checksum_without != checksum_with) {
        std::cerr << "benchmark paths produced different results\n";
        return 1;
    }

    const std::string record = "player=alice;level=12";
    const auto position = record.find('=');
    const std::string owning_copy = record.substr(0, position);
    const std::string_view non_owning_view(record.data(), position);
    if (owning_copy != non_owning_view) {
        return 1;
    }

    std::cout << "without reserve: " << without_ms << " ms\n";
    std::cout << "with reserve: " << with_ms << " ms\n";
    std::cout << "owning substring size=" << owning_copy.size()
              << ", view size=" << non_owning_view.size() << '\n';
    std::cout << "results are environment-dependent; repeat before drawing conclusions\n";
}
