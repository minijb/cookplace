#include <array>
#include <cassert>
#include <cstddef>
#include <charconv>
#include <iostream>
#include <memory_resource>
#include <string>
#include <system_error>
#include <type_traits>

int main() {
    static_assert(!std::is_same_v<std::string, std::pmr::string>);

    std::array<std::byte, 4096> arena{};
    std::pmr::monotonic_buffer_resource resource(
        arena.data(), arena.size(), std::pmr::null_memory_resource());

    {
        std::pmr::string combat_text(&resource);
        combat_text.reserve(512);
        combat_text.append("critical:");
        std::array<char, 32> number{};
        const auto conversion = std::to_chars(
            number.data(), number.data() + number.size(), 1200);
        assert(conversion.ec == std::errc{});
        combat_text.append(number.data(), conversion.ptr);
        combat_text.append(";target:");
        combat_text.append("slime");

        std::pmr::string status_text(&resource);
        status_text.assign("ready");

        assert(combat_text == "critical:1200;target:slime");
        assert(status_text == "ready");
        std::cout << combat_text << ", " << status_text << '\n';
    }

    resource.release();
    std::cout << "frame-local strings destroyed; arena released\n";
}
