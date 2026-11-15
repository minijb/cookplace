#include "ctest_demo/twice.h"

#include <cstdio>
#include <cstdlib>
#include <string>

int main() {
    // Reads a data file via a relative path: requires add_test WORKING_DIRECTORY
// to point at the source root (default would be the build directory).
    std::FILE* f = std::fopen("data/answer.txt", "r");
    if (!f) {
        std::printf("FAIL: data/answer.txt not found (check WORKING_DIRECTORY)\n");
        return 1;
    }
    int answer = 0;
    if (std::fscanf(f, "%d", &answer) != 1) {
        std::printf("FAIL: cannot parse data/answer.txt\n");
        return 1;
    }
    std::fclose(f);
    if (ctest_demo::twice(answer) != 84) {
        std::printf("FAIL: twice(%d) != 84\n", answer);
        return 1;
    }

    // Reads the variable injected by the ENVIRONMENT test property.
    const char* lang = std::getenv("CTEST_DEMO_LANG");
    if (lang == nullptr || std::string(lang) != "C++") {
        std::printf("FAIL: CTEST_DEMO_LANG is not 'C++'\n");
        return 1;
    }
    return 0;
}
