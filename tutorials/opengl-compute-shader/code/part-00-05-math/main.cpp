#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {

void expectNear(float actual, float expected, const char* label, float epsilon = 0.0001F) {
    if (std::abs(actual - expected) > epsilon) {
        std::fprintf(stderr, "%s: expected %.6f, got %.6f\n", label, expected, actual);
        throw std::runtime_error("math check failed");
    }
}

void expectVec3(const glm::vec3& actual, const glm::vec3& expected, const char* label) {
    expectNear(actual.x, expected.x, label);
    expectNear(actual.y, expected.y, label);
    expectNear(actual.z, expected.z, label);
}

void printVec3(const char* label, const glm::vec3& value) {
    std::printf("%s: (%.3f, %.3f, %.3f)\n", label, value.x, value.y, value.z);
}

} // namespace

int main() {
    try {
        const glm::vec3 a{1.0F, 0.0F, 0.0F};
        const glm::vec3 b{0.0F, 1.0F, 0.0F};

        expectVec3(a + b, {1.0F, 1.0F, 0.0F}, "vector addition");
        expectNear(glm::dot(a, b), 0.0F, "perpendicular dot");
        expectVec3(glm::cross(a, b), {0.0F, 0.0F, 1.0F}, "right-handed cross");

        const glm::vec3 normalized = glm::normalize(glm::vec3{3.0F, 4.0F, 0.0F});
        expectNear(glm::length(normalized), 1.0F, "normalized length");
        expectVec3(normalized, {0.6F, 0.8F, 0.0F}, "normalized value");
        printVec3("normalize(3, 4, 0)", normalized);

        const glm::mat4 translation =
            glm::translate(glm::mat4{1.0F}, glm::vec3{10.0F, 20.0F, 30.0F});
        const glm::vec4 movedPoint = translation * glm::vec4{1.0F, 2.0F, 3.0F, 1.0F};
        const glm::vec4 movedDirection = translation * glm::vec4{1.0F, 2.0F, 3.0F, 0.0F};
        expectVec3(glm::vec3{movedPoint}, {11.0F, 22.0F, 33.0F}, "translated point");
        expectVec3(glm::vec3{movedDirection}, {1.0F, 2.0F, 3.0F}, "unmoved direction");

        const glm::mat4 scale = glm::scale(glm::mat4{1.0F}, glm::vec3{2.0F});
        const glm::vec3 scaleThenTranslate =
            glm::vec3{translation * scale * glm::vec4{1.0F, 0.0F, 0.0F, 1.0F}};
        const glm::vec3 translateThenScale =
            glm::vec3{scale * translation * glm::vec4{1.0F, 0.0F, 0.0F, 1.0F}};
        expectVec3(scaleThenTranslate, {12.0F, 20.0F, 30.0F}, "scale then translate");
        expectVec3(translateThenScale, {22.0F, 40.0F, 60.0F}, "translate then scale");
        printVec3("T * S * point", scaleThenTranslate);
        printVec3("S * T * point", translateThenScale);

        const glm::mat4 rotateZ = glm::rotate(
            glm::mat4{1.0F}, glm::half_pi<float>(), glm::vec3{0.0F, 0.0F, 1.0F});
        const glm::vec3 rotated = glm::vec3{rotateZ * glm::vec4{1.0F, 0.0F, 0.0F, 1.0F}};
        expectVec3(rotated, {0.0F, 1.0F, 0.0F}, "rotate +X to +Y");
        printVec3("rotate +X by 90 degrees around +Z", rotated);

        std::puts("All chapter 05 math checks passed.");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
