#include "gl_common.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "texture_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
out vec2 vUV;
void main() { gl_Position = vec4(aPos, 1.0); vUV = aUV; }
)glsl";
const char* fragmentSource = R"glsl(#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTexture;
void main() { FragColor = texture(uTexture, vUV); }
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_08_textures [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.08 - Texture sampling", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;
    const GLuint texture = tutorial::loadTexture(std::string(PART01_ASSET_DIR) + "/checker.ppm");
    if (!texture) return 1;

    const float vertices[] = {
        -0.7F,-0.6F,0.0F, 0.0F,0.0F,
         0.7F,-0.6F,0.0F, 2.0F,0.0F,
         0.7F, 0.6F,0.0F, 2.0F,2.0F,
        -0.7F, 0.6F,0.0F, 0.0F,2.0F,
    };
    const unsigned int indices[] = {0,1,2, 2,3,0};
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUseProgram(program);
    tutorial::setInt(program, "uTexture", 0);
    std::puts("F: nearest/linear; W: repeat/clamp; Esc: exit");
    bool linear = true, repeat = true, previousF = false, previousW = false;
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        const bool pressedF = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        const bool pressedW = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        if (pressedF && !previousF) linear = !linear;
        if (pressedW && !previousW) repeat = !repeat;
        previousF = pressedF; previousW = pressedW;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glClearColor(0.06F, 0.07F, 0.10F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &ebo); glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
