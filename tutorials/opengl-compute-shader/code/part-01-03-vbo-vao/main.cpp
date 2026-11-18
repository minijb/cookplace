#include "gl_common.hpp"

namespace {
const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)glsl";
const char* fragmentSource = R"glsl(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(0.20, 0.75, 0.85, 1.0); }
)glsl";
}

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part01_03_vbo_vao [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.03 - VBO, VAO and attributes", hidden);
    if (!window) return 1;
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;

    const float vertices[] = {
        -0.6F, -0.5F, 0.0F,
         0.6F, -0.5F, 0.0F,
         0.0F,  0.6F, 0.0F,
    };
    // 1. 创建对象：VBO 保存顶点字节，VAO 保存顶点属性格式与读取状态。
    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    // 2. 先绑定 VAO，后续属性设置才会记录到这个 VAO。
    glBindVertexArray(vao);

    // 3. 再绑定 VBO；glBufferData 会为当前 GL_ARRAY_BUFFER 分配存储并复制 CPU 数据。
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 4. 描述 location 0：从当前 VBO 的 Offset 0 开始，每隔 12 字节读取 3 个 float。
    // 此调用会把属性格式和 VBO 引用记录到当前 VAO。
    glVertexAttribPointer(
        0,                         // 对应 GLSL：layout(location = 0) in vec3 aPos
        3,                         // x、y、z 三个分量
        GL_FLOAT,                  // 分量类型
        GL_FALSE,                  // 不进行整数归一化
        3 * sizeof(float),         // Stride：相邻顶点起点的字节距离
        reinterpret_cast<void*>(0) // Offset：属性在顶点记录内的字节偏移
    );

    // 5. 在当前 VAO 中启用属性 0；只设置 Pointer 而不启用不会按预期读取 VBO。
    glEnableVertexAttribArray(0);

    // 6. 初始化完成后解绑，避免后续代码意外修改状态。
    // 解绑不会删除对象，也不会清除 VAO 已保存的属性格式和 VBO 引用。
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        glClearColor(0.07F, 0.08F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        // 每帧只需选择 Program 和 VAO；VAO 会恢复属性 0 的格式、启用状态和 VBO 引用。
        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3); // 从顶点 0 开始读取 3 个顶点，组成一个三角形
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // 解绑不等于删除；程序结束且 Context 仍有效时，分别释放 GPU 对象。
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
