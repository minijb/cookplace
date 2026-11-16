#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace {

const char* debugSourceToString(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window system";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third party";
        case GL_DEBUG_SOURCE_APPLICATION: return "Application";
        case GL_DEBUG_SOURCE_OTHER: return "Other";
        default: return "Unknown";
    }
}

const char* debugTypeToString(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined behavior";
        case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
        case GL_DEBUG_TYPE_MARKER: return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP: return "Push group";
        case GL_DEBUG_TYPE_POP_GROUP: return "Pop group";
        case GL_DEBUG_TYPE_OTHER: return "Other";
        default: return "Unknown";
    }
}

const char* debugSeverityToString(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: return "High";
        case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
        case GL_DEBUG_SEVERITY_LOW: return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
        default: return "Unknown";
    }
}

void APIENTRY debugCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei,
    const GLchar* message,
    const void*) {
    std::fprintf(
        stderr,
        "[OpenGL %s | %s | %s | id=%u] %s\n",
        debugSeverityToString(severity),
        debugSourceToString(source),
        debugTypeToString(type),
        id,
        message);
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

} // namespace

int main(int argc, char** argv) {
    bool hidden = false;
    bool triggerError = false;
    int maxFrames = -1;
    for (int i = 1; i < argc; ++i) {
        const std::string_view argument = argv[i];
        if (argument == "--hidden") {
            hidden = true;
        } else if (argument == "--trigger-error") {
            triggerError = true;
        } else if (argument == "--max-frames" && i + 1 < argc) {
            maxFrames = std::max(1, std::atoi(argv[++i]));
        } else {
            std::fprintf(
                stderr,
                "Usage: ch07_debug [--hidden] [--max-frames N] [--trigger-error]\n");
            return 2;
        }
    }

    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        return 1;
    }

    // Debug Output 在 OpenGL 4.3 进入 Core；本课直接请求 4.3 Core + Debug Context。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, hidden ? GLFW_FALSE : GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Part 00 - Debug", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "An OpenGL 4.3 Core debug context is required\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    int major = 0;
    int minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    std::printf("Debug context version: %d.%d\n", major, minor);
    if (major < 4 || (major == 4 && minor < 3)) {
        std::fprintf(stderr, "OpenGL 4.3 is required for core Debug Output\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);

    glDebugMessageInsert(
        GL_DEBUG_SOURCE_APPLICATION,
        GL_DEBUG_TYPE_MARKER,
        1000,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        -1,
        "Debug output is connected");

    GLuint buffer = 0;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 64, nullptr, GL_DYNAMIC_DRAW);
    glObjectLabel(GL_BUFFER, buffer, -1, "Part 00 demonstration buffer");

    if (triggerError) {
        std::puts("Triggering a deliberate GL_INVALID_ENUM error");
        glEnable(0); // 故意传入非法枚举，观察回调如何报告错误。
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 2000, -1, "Frame");
        glClearColor(0.12F, 0.08F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glPopDebugGroup();
        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frame;
        if (maxFrames > 0 && frame >= maxFrames) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    glDeleteBuffers(1, &buffer);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
