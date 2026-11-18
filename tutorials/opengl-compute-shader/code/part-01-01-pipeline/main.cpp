#include "gl_common.hpp"

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_01_pipeline [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.01 - OpenGL state machine", hidden);
    if (!window) return 1;

    std::puts("1/2/3: select clear color; Esc: exit");
    int selectedColor = 0;
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) selectedColor = 0;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) selectedColor = 1;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) selectedColor = 2;
        const glm::vec3 colors[] = {
            {0.08F, 0.10F, 0.16F}, {0.16F, 0.05F, 0.08F}, {0.04F, 0.15F, 0.11F}
        };
        const glm::vec3 color = colors[selectedColor];
        glClearColor(color.r, color.g, color.b, 1.0F); // 设置状态
        glClear(GL_COLOR_BUFFER_BIT);                  // 使用状态清除颜色附件
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
