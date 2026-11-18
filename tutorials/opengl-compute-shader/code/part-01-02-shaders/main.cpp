#include "gl_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
// 仅本课使用 gl_VertexID 生成位置，把注意力集中在 Shader 编译与链接。
const vec2 positions[3] = vec2[](vec2(-0.6, -0.5), vec2(0.6, -0.5), vec2(0.0, 0.6));
void main() {
    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}
)glsl";
const char* fragmentSource = R"glsl(#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.95, 0.45, 0.20, 1.0);
}
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_02_shaders [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.02 - Shader compilation and linking", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao); // Core Profile 绘制时需要一个 VAO，即使本例不读取属性。

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        glClearColor(0.07F, 0.08F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
