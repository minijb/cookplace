#pragma once
#include <cmath>

namespace vec {
struct Vec2 {
    float x, y;
    float dot(Vec2 o) const { return x * o.x + y * o.y; }
    float len() const { return std::sqrt(dot(*this)); }
};
}
