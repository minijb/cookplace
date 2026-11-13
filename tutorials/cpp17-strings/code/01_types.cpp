#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

std::size_t text_length(std::string_view text) {
    return text.size();
}

bool has_prefix(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() &&
           text.substr(0, prefix.size()) == prefix;
}

void rename_player(std::string& player_name, std::string_view new_name) {
    player_name.assign(new_name.data(), new_name.size());
}

struct PlayerProfile {
    std::string name;
};

int main() {
    const char* literal = "player";
    const std::string owned = "alice";
    const std::string_view borrowed = owned;

    assert(text_length(literal) == 6);
    assert(text_length(borrowed) == 5);
    assert(has_prefix(owned, "ali"));

    std::string player_name = "old-name";
    rename_player(player_name, borrowed);
    assert(player_name == "alice");

    PlayerProfile profile{std::string(borrowed)};
    assert(profile.name == owned);

    const std::string embedded_null("a\0b", 3);
    assert(embedded_null.size() == 3);

    std::cout << "owned=" << owned << ", borrowed=" << borrowed
              << ", embedded_null_size=" << embedded_null.size() << '\n';
}
