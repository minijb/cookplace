#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {

void expectNear(float actual, float expected, const char* label, float epsilon = 0.0001F) {
    if (std::abs(actual - expected) > epsilon) {
        std::fprintf(stderr, "%s: expected %.6f, got %.6f\n", label, expected, actual);
        throw std::runtime_error("coordinate check failed");
    }
}

void expectVec3(const glm::vec3& actual, const glm::vec3& expected, const char* label) {
    expectNear(actual.x, expected.x, label);
    expectNear(actual.y, expected.y, label);
    expectNear(actual.z, expected.z, label);
}

void printVec3(const char* label, const glm::vec3& value) {
    std::printf("%s: (%.6f, %.6f, %.6f)\n", label, value.x, value.y, value.z);
}

glm::vec3 ndcFromClip(const glm::vec4& clip) {
    return glm::vec3{clip} / clip.w;
}

glm::vec2 ndcToWindow(const glm::vec2& ndc, float width, float height) {
    return (ndc + glm::vec2{1.0F}) * 0.5F * glm::vec2{width, height};
}

glm::vec2 windowToNdc(const glm::vec2& window, float width, float height) {
    return window / glm::vec2{width, height} * 2.0F - glm::vec2{1.0F};
}

} // namespace

int main() {
    try {
        expectVec3(
            glm::cross(glm::vec3{1.0F, 0.0F, 0.0F}, glm::vec3{0.0F, 1.0F, 0.0F}),
            {0.0F, 0.0F, 1.0F},
            "right-handed basis");

        const glm::mat4 model =
            glm::translate(glm::mat4{1.0F}, glm::vec3{2.0F, 0.0F, 0.0F});
        const glm::vec4 localPosition{1.0F, 0.0F, 0.0F, 1.0F};
        const glm::vec4 worldPosition = model * localPosition;
        expectVec3(glm::vec3{worldPosition}, {3.0F, 0.0F, 0.0F}, "local to world");

        const glm::mat4 view = glm::lookAtRH(
            glm::vec3{0.0F, 0.0F, 3.0F},
            glm::vec3{0.0F, 0.0F, 0.0F},
            glm::vec3{0.0F, 1.0F, 0.0F});
        const glm::vec4 viewOrigin = view * glm::vec4{0.0F, 0.0F, 0.0F, 1.0F};
        expectVec3(glm::vec3{viewOrigin}, {0.0F, 0.0F, -3.0F}, "world to view");
        printVec3("World origin in view space", glm::vec3{viewOrigin});

        constexpr float nearPlane = 0.1F;
        constexpr float farPlane = 100.0F;
        const glm::mat4 projection = glm::perspectiveRH_NO(
            glm::radians(45.0F), 800.0F / 600.0F, nearPlane, farPlane);

        const glm::vec4 clipOrigin = projection * viewOrigin;
        expectNear(clipOrigin.w, 3.0F, "perspective w equals view distance");
        expectVec3(ndcFromClip(clipOrigin), {0.0F, 0.0F, 0.9353F}, "clip to NDC");

        const glm::vec4 nearClip = projection * glm::vec4{0.0F, 0.0F, -nearPlane, 1.0F};
        const glm::vec4 farClip = projection * glm::vec4{0.0F, 0.0F, -farPlane, 1.0F};
        expectNear(ndcFromClip(nearClip).z, -1.0F, "near plane NDC");
        expectNear(ndcFromClip(farClip).z, 1.0F, "far plane NDC");

        const glm::vec3 nearSide = ndcFromClip(
            projection * view * glm::vec4{1.0F, 0.0F, 0.0F, 1.0F});
        const glm::vec3 farSide = ndcFromClip(
            projection * view * glm::vec4{1.0F, 0.0F, -3.0F, 1.0F});
        if (!(std::abs(farSide.x) < std::abs(nearSide.x))) {
            throw std::runtime_error("perspective should shrink distant X coordinates");
        }
        printVec3("Near point NDC", nearSide);
        printVec3("Far point NDC", farSide);

        const glm::vec2 lowerLeft = ndcToWindow({-1.0F, -1.0F}, 800.0F, 600.0F);
        const glm::vec2 upperRight = ndcToWindow({1.0F, 1.0F}, 800.0F, 600.0F);
        const glm::vec2 center = ndcToWindow({0.0F, 0.0F}, 800.0F, 600.0F);
        expectNear(lowerLeft.x, 0.0F, "viewport lower-left x");
        expectNear(lowerLeft.y, 0.0F, "viewport lower-left y");
        expectNear(upperRight.x, 800.0F, "viewport upper-right x");
        expectNear(upperRight.y, 600.0F, "viewport upper-right y");
        expectNear(center.x, 400.0F, "viewport center x");
        expectNear(center.y, 300.0F, "viewport center y");
        const glm::vec2 centerNdc = windowToNdc(center, 800.0F, 600.0F);
        expectNear(centerNdc.x, 0.0F, "window center back to NDC x");
        expectNear(centerNdc.y, 0.0F, "window center back to NDC y");

        // 与正文一致的完整数值示例：Local → World → View → Clip → NDC → Window。
        const glm::mat4 workedModel =
            glm::translate(glm::mat4{1.0F}, glm::vec3{0.5F, 0.0F, 0.0F});
        const glm::vec4 workedLocal{0.5F, 0.0F, 0.0F, 1.0F};
        const glm::vec4 workedWorld = workedModel * workedLocal;
        expectVec3(glm::vec3{workedWorld}, {1.0F, 0.0F, 0.0F}, "worked world position");
        const glm::vec4 workedView = view * workedWorld;
        expectVec3(glm::vec3{workedView}, {1.0F, 0.0F, -3.0F}, "worked view position");
        const glm::vec4 workedClip = projection * workedView;
        expectNear(workedClip.x, 1.8107F, "worked clip x");
        expectNear(workedClip.z, 2.8058F, "worked clip z");
        expectNear(workedClip.w, 3.0F, "worked clip w");
        const glm::vec3 workedNdc = ndcFromClip(workedClip);
        expectVec3(workedNdc, {0.6036F, 0.0F, 0.9353F}, "worked NDC");
        const glm::vec2 workedWindow = ndcToWindow(
            glm::vec2{workedNdc}, 800.0F, 600.0F);
        expectNear(workedWindow.x, 641.4213F, "worked window x", 0.001F);
        expectNear(workedWindow.y, 300.0F, "worked window y");
        const glm::vec2 workedBackToNdc = windowToNdc(workedWindow, 800.0F, 600.0F);
        expectNear(workedBackToNdc.x, workedNdc.x, "worked inverse viewport x");
        expectNear(workedBackToNdc.y, workedNdc.y, "worked inverse viewport y");
        printVec3("Worked example NDC", workedNdc);
        std::printf(
            "Worked example window: (%.4f, %.4f)\n", workedWindow.x, workedWindow.y);

        std::printf("NDC (0, 0) maps to window (%.1f, %.1f)\n", center.x, center.y);
        std::puts("All chapter 06 coordinate checks passed.");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
