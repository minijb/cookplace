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
uniform sampler2D uChecker;
uniform sampler2D uGradient;
uniform float uMix;
void main() {
    vec4 a = texture(uChecker, vUV);
    vec4 b = texture(uGradient, vUV);
    FragColor = mix(a, b, uMix);
}
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_09_multitexture [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.09 - Two texture units", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    const std::string assetDir = PART01_ASSET_DIR;
    const GLuint checker = tutorial::loadTexture(assetDir + "/checker.ppm");
    const GLuint gradient = tutorial::loadTexture(assetDir + "/gradient.ppm");
    if (!program || !checker || !gradient) return 1;
    const float vertices[] = {
        -0.75F,-0.6F,0.0F, 0.0F,0.0F,  0.75F,-0.6F,0.0F, 1.0F,0.0F,
         0.75F, 0.6F,0.0F, 1.0F,1.0F, -0.75F, 0.6F,0.0F, 0.0F,1.0F,
    };
    const unsigned int indices[] = {0,1,2, 2,3,0};
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glUseProgram(program);
    tutorial::setInt(program, "uChecker", 0);
    tutorial::setInt(program, "uGradient", 1);
    std::puts("Up/Down: change blend factor; Esc: exit");
    float blend = 0.35F;
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) blend += 0.01F;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) blend -= 0.01F;
        blend = glm::clamp(blend, 0.0F, 1.0F);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, checker);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gradient);
        glClearColor(0.06F, 0.07F, 0.10F, 1.0F); glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program); tutorial::setFloat(program, "uMix", blend);
        glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window); glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glDeleteTextures(1, &checker); glDeleteTextures(1, &gradient);
    glDeleteBuffers(1, &ebo); glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
