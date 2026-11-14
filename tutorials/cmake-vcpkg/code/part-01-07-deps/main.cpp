#include <fmt/format.h>

int main() {
    fmt::print("Hello, {}! (fmt {})\n", "FetchContent", FMT_VERSION);
    return 0;
}
