#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace {

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

} // namespace

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    for (int i = 1; i < argc; ++i) {
        const std::string_view argument = argv[i];
        if (argument == "--hidden") {
            hidden = true;
        } else if (argument == "--max-frames" && i + 1 < argc) {
            maxFrames = std::max(1, std::atoi(argv[++i]));
        } else {
            std::fprintf(stderr, "Usage: ch04_project [--hidden] [--max-frames N]\n");
            return 2;
        }
    }

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Part 00 - Project", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    std::printf("Project configured with CMake + vcpkg\n");
    std::printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        glClearColor(0.10F, 0.14F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frame;
        if (maxFrames > 0 && frame >= maxFrames) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
