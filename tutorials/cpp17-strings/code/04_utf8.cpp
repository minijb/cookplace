#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

std::size_t count_code_points_in_valid_utf8(std::string_view text) {
    std::size_t count = 0;
    for (const unsigned char byte : text) {
        if ((byte & 0xc0u) != 0x80u) {
            ++count;
        }
    }
    return count;
}

int main() {
    const std::string ascii = "A";
    const std::string cjk = u8"界";
    const std::string combined = u8"A界";
    const std::string combining = u8"é";

    assert(ascii.size() == 1);
    assert(cjk.size() == 3);
    assert(combined.size() == 4);
    assert(count_code_points_in_valid_utf8(combined) == 2);

    assert(combining.size() == 3);
    assert(count_code_points_in_valid_utf8(combining) == 2);

    std::cout << "A: bytes=" << ascii.size() << '\n';
    std::cout << "CJK: bytes=" << cjk.size() << ", code_points=1\n";
    std::cout << "A+CJK: bytes=" << combined.size() << ", code_points="
              << count_code_points_in_valid_utf8(combined) << '\n';
    std::cout << "e + combining acute: bytes=" << combining.size()
              << ", code_points="
              << count_code_points_in_valid_utf8(combining)
              << ", user_perceived_characters=1\n";
}
