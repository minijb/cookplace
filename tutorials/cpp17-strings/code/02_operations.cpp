#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

int main() {
    const std::string record = "player=alice;level=12";
    const auto separator = record.find('=');
    assert(separator != std::string::npos);

    const std::string key = record.substr(0, separator);
    const auto end = record.find(';', separator + 1);
    const std::string value = record.substr(separator + 1, end - separator - 1);
    assert(key == "player");
    assert(value == "alice");

    assert(record.find("missing") == std::string::npos);
    assert(record.compare(0, 6, "player") == 0);

    std::string message = "score:12";
    message.insert(message.size(), ";combo:2");
    assert(message == "score:12;combo:2");
    message.erase(message.find(";combo"));
    assert(message == "score:12");
    message.replace(message.find("12"), 2, "20");
    assert(message == "score:20");

    bool caught = false;
    try {
        static_cast<void>(message.at(message.size()));
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    const std::string bytes("a\0b", 3);
    assert(bytes.size() == 3);
    assert(bytes.find(std::string_view("b", 1)) == 2);

    std::cout << key << '=' << value << ", message=" << message << '\n';
}
