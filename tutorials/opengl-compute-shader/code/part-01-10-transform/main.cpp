#include "gl_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vColor;
uniform mat4 uModel;
void main() { gl_Position = uModel * vec4(aPos, 1.0); vColor = aColor; }
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
            "Usage: part01_10_transform [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.10 - Transform order (Space: pause)", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;
    const float vertices[] = {
        -0.5F,-0.35F,0.0F, 0.95F,0.30F,0.20F,  0.5F,-0.35F,0.0F, 0.20F,0.80F,0.45F,
         0.5F, 0.35F,0.0F, 0.25F,0.45F,0.95F, -0.5F, 0.35F,0.0F, 0.95F,0.75F,0.20F,
    };
    const unsigned int indices[] = {0,1,2, 2,3,0};
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float))); glEnableVertexAttribArray(1);
    bool paused = false, previousSpace = false;
    float simulationTime = 0.0F;
    double previousTime = glfwGetTime();
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float delta = static_cast<float>(now - previousTime);
        previousTime = now;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        const bool space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (space && !previousSpace) paused = !paused;
        previousSpace = space;
        if (!paused) simulationTime += delta;
        // 矩阵乘列向量时，最右侧的缩放先执行，然后旋转，最后平移。
        glm::mat4 model{1.0F};
        model = glm::translate(model, glm::vec3{0.25F, 0.10F, 0.0F});
        model = glm::rotate(model, simulationTime, glm::vec3{0.0F, 0.0F, 1.0F});
        model = glm::scale(model, glm::vec3{0.8F, 0.8F, 1.0F});
        glClearColor(0.06F, 0.07F, 0.10F, 1.0F); glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program); tutorial::setMat4(program, "uModel", model);
        glBindVertexArray(vao); glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glfwSwapBuffers(window); glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glDeleteBuffers(1, &ebo); glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao); glDeleteProgram(program);
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
