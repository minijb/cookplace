#include "ctest_demo/twice.h"

#include <cstdio>

// Prefer returning nonzero over <cassert>: MSVC Debug assert pops a dialog
// box, which stalls unattended CI runs.
#define CHECK(expr)                                                     \
    do {                                                                \
        if (!(expr)) {                                                  \
            std::printf("FAIL: %s (%s:%d)\n", #expr, __FILE__, __LINE__); \
            return 1;                                                   \
        }                                                               \
    } while (0)

int main() {
    CHECK(ctest_demo::twice(2) == 4);
    CHECK(ctest_demo::twice(-3) == -6);
    return 0;
}
