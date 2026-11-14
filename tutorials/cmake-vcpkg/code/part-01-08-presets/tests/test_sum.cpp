#include "presets/twice.h"
#include <cassert>

int main() {
    assert(presets::twice(2) == 4);
    assert(presets::twice(-3) == -6);
    return 0;
}
