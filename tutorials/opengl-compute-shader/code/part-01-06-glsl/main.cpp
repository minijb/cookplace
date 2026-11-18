#include "gl_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vColor;
out vec2 vPosition;
void main() {
    gl_Position = vec4(aPos, 1.0);
    vColor = aColor;
    vPosition = aPos.xy;
}
)glsl";
const char* fragmentSource = R"glsl(#version 330 core
in vec3 vColor;
in vec2 vPosition;
out vec4 FragColor;
void main() {
    // 标量、向量、分量访问、构造函数与常用内置函数。
    float wave = 0.5 + 0.5 * sin(14.0 * vPosition.x);
    float band = smoothstep(0.25, 0.75, wave);
    vec3 coolColor = vec3(0.08, 0.16, 0.30);
    vec3 rgb = mix(vColor, coolColor, 0.35 * band);
    rgb = clamp(rgb, 0.0, 1.0);
    FragColor = vec4(rgb, 1.0);
}
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_06_glsl [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.06 - GLSL types and built-in functions", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;
    const float vertices[] = {
        -0.7F, -0.55F, 0.0F, 0.95F, 0.30F, 0.20F,
         0.7F, -0.55F, 0.0F, 0.20F, 0.85F, 0.45F,
         0.0F,  0.65F, 0.0F, 0.30F, 0.50F, 1.00F,
    };
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        glClearColor(0.05F, 0.06F, 0.09F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
