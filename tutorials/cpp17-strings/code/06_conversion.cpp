#include <array>
#include <cassert>
#include <charconv>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>

std::string build_player_label(std::string_view name, int score) {
    std::array<char, 32> number{};
    const auto conversion = std::to_chars(
        number.data(), number.data() + number.size(), score);
    assert(conversion.ec == std::errc{});

    std::string result;
    result.reserve(name.size() + 14 +
                   static_cast<std::size_t>(conversion.ptr - number.data()));
    result.append("player:");
    result.append(name.data(), name.size());
    result.append(" score:");
    result.append(number.data(), conversion.ptr);
    return result;
}

bool parse_score(std::string_view text, int& score) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), score);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

int main() {
    const std::string label = build_player_label("alice", 1200);
    assert(label == "player:alice score:1200");

    int score = 0;
    assert(parse_score("1200", score));
    assert(score == 1200);
    assert(!parse_score("1200 points", score));
    assert(!parse_score("not-a-number", score));

    std::ostringstream stream;
    stream << "player:" << "alice" << " score:" << 1200;
    assert(stream.str() == label);

    std::cout << label << '\n';
}
