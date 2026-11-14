#include "vec/vec2.h"
#include <cstdio>

int main() {
    vec::Vec2 v{3.0f, 4.0f};
    std::printf("len = %.1f\n", v.len());
    return 0;
}
