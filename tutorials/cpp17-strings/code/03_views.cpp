#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::string_view trim_ascii(std::string_view text) {
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.front())) != 0) {
        text.remove_prefix(1);
    }
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.back())) != 0) {
        text.remove_suffix(1);
    }
    return text;
}

std::vector<std::string_view> split_view(std::string_view text, char separator) {
    std::vector<std::string_view> parts;
    std::size_t begin = 0;
    while (begin <= text.size()) {
        const auto end = text.find(separator, begin);
        if (end == std::string_view::npos) {
            parts.push_back(text.substr(begin));
            break;
        }
        parts.push_back(text.substr(begin, end - begin));
        begin = end + 1;
    }
    return parts;
}

int main() {
    const std::string input = "  alice,level-12,region-asia  ";
    const std::string_view cleaned = trim_ascii(input);
    const auto parts = split_view(cleaned, ',');

    assert(parts.size() == 3);
    assert(parts[0] == "alice");
    assert(parts[1] == "level-12");
    assert(parts[2] == "region-asia");

    const std::string storage = "xxplayerYY";
    const std::string_view view(storage.data() + 2, 6);
    assert(view == "player");
    assert(view.size() == 6);

    const std::string saved_for_later(parts[1]);
    assert(saved_for_later == "level-12");

    std::cout << "parts=" << parts.size() << ", saved=" << saved_for_later
              << ", view=" << view << '\n';
}
