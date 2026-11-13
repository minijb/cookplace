#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>

int main() {
    std::string text;
    text.reserve(1024);
    const auto reserved_capacity = text.capacity();
    assert(text.size() == 0);
    assert(reserved_capacity >= 1024);

    const char* original_data = text.data();
    while (text.size() < 1024) {
        text.push_back('x');
    }
    assert(text.size() == 1024);
    assert(text.capacity() == reserved_capacity);
    assert(text.data() == original_data);

    text.resize(16, 'a');
    assert(text.size() == 16);
    assert(text[15] == 'x');
    text.resize(20, 'a');
    assert(text.size() == 20);
    assert(text[19] == 'a');
    text.shrink_to_fit();
    assert(text.size() == 20);

    std::string source(4096, 'x');
    std::string destination = std::move(source);
    assert(destination.size() == 4096);
    source = "reused";
    assert(source == "reused");

    std::string changing = "short";
    const char* before = changing.data();
    changing.reserve(8192);
    const char* after = changing.data();
    std::cout << "pointer_changed_after_growth=" << (before != after) << '\n';

    std::cout << "sizeof(string)=" << sizeof(std::string)
              << ", size=" << text.size() << ", capacity=" << text.capacity()
              << ", moved_size=" << destination.size() << '\n';
}
