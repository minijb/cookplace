#include "gl_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
out vec3 vColor;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    vColor = vec3(aUV, 0.65) * (0.65 + 0.35 * aPos.y);
}
)glsl";
const char* fragmentSource = R"glsl(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() { FragColor = vec4(vColor, 1.0); }
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_11_mvp [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.11 - Model View Projection", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;
    tutorial::CubeGeometry cube;
    glEnable(GL_DEPTH_TEST); // 先保证立方体遮挡正确；第 13 课详细讨论深度与面剔除。
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        const float aspect = static_cast<float>(width) / std::max(height, 1);
        const float time = static_cast<float>(glfwGetTime());
        glm::mat4 model{1.0F};
        model = glm::rotate(model, time * 0.6F, glm::normalize(glm::vec3{0.4F, 1.0F, 0.2F}));
        const glm::mat4 view = glm::lookAt(glm::vec3{3.0F, 2.0F, 5.0F}, glm::vec3{0.0F}, glm::vec3{0.0F, 1.0F, 0.0F});
        const glm::mat4 projection = glm::perspective(glm::radians(45.0F), aspect, 0.1F, 100.0F);
        glClearColor(0.05F, 0.06F, 0.10F, 1.0F); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);
        tutorial::setMat4(program, "uModel", model);
        tutorial::setMat4(program, "uView", view);
        tutorial::setMat4(program, "uProjection", projection);
        cube.draw();
        glfwSwapBuffers(window); glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    cube.destroy(); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
