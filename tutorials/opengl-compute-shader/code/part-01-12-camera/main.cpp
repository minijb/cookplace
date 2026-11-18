#include "gl_common.hpp"

namespace {
struct Camera {
    glm::vec3 position{0.0F, 0.0F, 6.0F};
    glm::vec3 front{0.0F, 0.0F, -1.0F};
    glm::vec3 up{0.0F, 1.0F, 0.0F};
    float yaw = -90.0F;
    float pitch = 0.0F;
};
Camera camera;
float lastX = 480.0F;
float lastY = 320.0F;
bool firstMouse = true;

void mouseCallback(GLFWwindow*, double x, double y) {
    if (firstMouse) { lastX = static_cast<float>(x); lastY = static_cast<float>(y); firstMouse = false; }
    const float dx = static_cast<float>(x) - lastX;
    const float dy = lastY - static_cast<float>(y);
    lastX = static_cast<float>(x); lastY = static_cast<float>(y);
    camera.yaw += dx * 0.12F;
    camera.pitch = glm::clamp(camera.pitch + dy * 0.12F, -89.0F, 89.0F);
    const float yaw = glm::radians(camera.yaw);
    const float pitch = glm::radians(camera.pitch);
    camera.front = glm::normalize(glm::vec3{std::cos(yaw) * std::cos(pitch), std::sin(pitch), std::sin(yaw) * std::cos(pitch)});
}

const char* vertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
out vec3 vColor;
uniform mat4 uModel, uView, uProjection;
void main() { gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0); vColor = vec3(aUV, 0.7); }
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
            "Usage: part01_12_camera [--hidden] [--max-frames N]")) return 2;
    GLFWwindow* window = tutorial::createWindow("Part 01.12 - FPS Camera", hidden);
    if (!window) return 1;
    if (!hidden) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouseCallback);
    }
    const GLuint program = tutorial::createProgram(vertexSource, fragmentSource);
    if (!program) return 1;
    tutorial::CubeGeometry cube;
    glEnable(GL_DEPTH_TEST);
    std::puts("WASD: move; mouse: look; Esc: exit");
    double previousTime = glfwGetTime();
    int frame = 0;
    while (!glfwWindowShouldClose(window)) {
        const double now = glfwGetTime();
        const float deltaTime = static_cast<float>(now - previousTime);
        previousTime = now;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        const float speed = 3.0F * deltaTime;
        const glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.position += speed * camera.front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.position -= speed * camera.front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.position -= speed * right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.position += speed * right;
        int width = 0, height = 0; glfwGetFramebufferSize(window, &width, &height);
        const glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        const glm::mat4 projection = glm::perspective(glm::radians(50.0F), static_cast<float>(width) / std::max(height, 1), 0.1F, 100.0F);
        glClearColor(0.05F, 0.06F, 0.10F, 1.0F); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program); tutorial::setMat4(program, "uView", view); tutorial::setMat4(program, "uProjection", projection);
        const glm::vec3 positions[] = {{0,0,0}, {2,0,-2}, {-2,0,-2}, {2,0,2}, {-2,0,2}};
        for (const glm::vec3& position : positions) {
            glm::mat4 model{1.0F}; model = glm::translate(model, position);
            tutorial::setMat4(program, "uModel", model); cube.draw();
        }
        glfwSwapBuffers(window); glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    cube.destroy(); glDeleteProgram(program); glfwDestroyWindow(window); glfwTerminate();
    return 0;
}
