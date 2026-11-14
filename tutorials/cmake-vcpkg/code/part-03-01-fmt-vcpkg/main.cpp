#include <fmt/color.h>
#include <fmt/format.h>

int main() {
    fmt::print(fg(fmt::color::cyan), "Hello, {}!\n", "vcpkg + fmt");
    return 0;
}
