#include "gl_common.hpp"

#include <glm/gtc/matrix_inverse.hpp>

#include <array>
#include <vector>

namespace {

const char* objectVertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 vWorldPos;
out vec3 vWorldNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vWorldNormal = uNormalMatrix * aNormal;
    gl_Position = uProjection * uView * worldPos;
}
)glsl";

const char* objectFragmentSource = R"glsl(#version 330 core
in vec3 vWorldPos;
in vec3 vWorldNormal;
out vec4 FragColor;

uniform vec3 uObjectColor;
uniform vec3 uLightColor;
uniform vec3 uLightPos;
uniform bool uShowNormals;

void main() {
    vec3 normal = normalize(vWorldNormal);
    if (uShowNormals) {
        FragColor = vec4(normal * 0.5 + 0.5, 1.0);
        return;
    }

    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    vec3 litColor = uObjectColor * uLightColor * diffuseFactor;
    FragColor = vec4(litColor, 1.0);
}
)glsl";

const char* lightVertexSource = R"glsl(#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
void main() {
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
)glsl";

const char* lightFragmentSource = R"glsl(#version 330 core
out vec4 FragColor;
uniform vec3 uLightColor;
void main() {
    FragColor = vec4(uLightColor, 1.0);
}
)glsl";

// 八面体的 8 个三角形面都具有斜法线，便于观察非均匀缩放下的 Normal Matrix 差异。
constexpr glm::vec3 corners[] = {
    {1.0F, 0.0F, 0.0F}, {-1.0F, 0.0F, 0.0F},
    {0.0F, 1.0F, 0.0F}, {0.0F, -1.0F, 0.0F},
    {0.0F, 0.0F, 1.0F}, {0.0F, 0.0F, -1.0F},
};

// 索引顺序从外部观察均为 CCW；法线由两条边的叉积计算。
constexpr int faces[][3] = {
    {2, 4, 0}, {2, 0, 5}, {2, 5, 1}, {2, 1, 4},
    {3, 0, 4}, {3, 5, 0}, {3, 1, 5}, {3, 4, 1},
};

class NormalOctahedron {
public:
    NormalOctahedron() {
        std::vector<std::array<float, 6>> vertices;
        vertices.reserve(24);
        for (const auto& face : faces) {
            const glm::vec3 p0 = corners[face[0]];
            const glm::vec3 p1 = corners[face[1]];
            const glm::vec3 p2 = corners[face[2]];
            const glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
            for (int cornerIndex : face) {
                const glm::vec3 p = corners[cornerIndex];
                vertices.push_back({p.x, p.y, p.z, normal.x, normal.y, normal.z});
            }
        }

        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertices.size() * sizeof(vertices[0])),
            vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
            reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    NormalOctahedron(const NormalOctahedron&) = delete;
    NormalOctahedron& operator=(const NormalOctahedron&) = delete;

    void draw() const {
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 24);
    }

    void destroy() {
        if (vbo_ == 0 && vao_ == 0) return;
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
        vbo_ = 0;
        vao_ = 0;
    }

    ~NormalOctahedron() { destroy(); }

private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};

void setMat3(GLuint program, const char* name, const glm::mat3& value) {
    glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace

int main(int argc, char** argv) {
    bool hidden = false;
    int maxFrames = -1;
    if (!tutorial::parseArguments(argc, argv, hidden, maxFrames,
            "Usage: part02_01_lighting_basics [--hidden] [--max-frames N]")) return 2;

    GLFWwindow* window = tutorial::createWindow("Part 02.01 - Colors, normals and diffuse light", hidden);
    if (!window) return 1;

    const GLuint objectProgram = tutorial::createProgram(objectVertexSource, objectFragmentSource);
    const GLuint lightProgram = tutorial::createProgram(lightVertexSource, lightFragmentSource);
    if (!objectProgram || !lightProgram) {
        glDeleteProgram(objectProgram);
        glDeleteProgram(lightProgram);
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    NormalOctahedron mesh;
    const glm::vec3 objectColor{1.0F, 0.5F, 0.31F};
    const glm::vec3 lightColor{1.0F, 1.0F, 1.0F};
    const glm::vec3 lightPos{2.5F, 2.0F, 3.0F};
    const glm::vec3 cameraPos{3.0F, 2.0F, 5.0F};

    glUseProgram(objectProgram);
    tutorial::setVec3(objectProgram, "uObjectColor", objectColor);
    tutorial::setVec3(objectProgram, "uLightColor", lightColor);
    tutorial::setVec3(objectProgram, "uLightPos", lightPos);
    glUseProgram(lightProgram);
    tutorial::setVec3(lightProgram, "uLightColor", lightColor);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    std::puts("N: toggle correct normal matrix; V: visualize normals; Esc: exit");
    bool correctNormalMatrix = true;
    bool showNormals = false;
    bool previousN = false;
    bool previousV = false;
    int frame = 0;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        const bool n = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
        const bool v = glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS;
        if (n && !previousN) {
            correctNormalMatrix = !correctNormalMatrix;
            std::printf("Normal matrix: %s\n", correctNormalMatrix ? "inverse-transpose" : "mat3(model)");
        }
        if (v && !previousV) showNormals = !showNormals;
        previousN = n;
        previousV = v;

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        const glm::mat4 view = glm::lookAt(cameraPos, glm::vec3{0.0F}, glm::vec3{0.0F, 1.0F, 0.0F});
        const glm::mat4 projection = glm::perspective(
            glm::radians(45.0F), static_cast<float>(width) / std::max(height, 1), 0.1F, 100.0F);

        glClearColor(0.04F, 0.05F, 0.08F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float angle = static_cast<float>(glfwGetTime()) * 0.35F;
        glm::mat4 model{1.0F};
        model = glm::rotate(model, angle, glm::vec3{0.0F, 1.0F, 0.0F});
        model = glm::scale(model, glm::vec3{1.4F, 0.65F, 1.0F});
        const glm::mat3 normalMatrix = correctNormalMatrix
            ? glm::transpose(glm::inverse(glm::mat3(model)))
            : glm::mat3(model);

        glUseProgram(objectProgram);
        tutorial::setMat4(objectProgram, "uModel", model);
        tutorial::setMat4(objectProgram, "uView", view);
        tutorial::setMat4(objectProgram, "uProjection", projection);
        setMat3(objectProgram, "uNormalMatrix", normalMatrix);
        tutorial::setInt(objectProgram, "uShowNormals", showNormals ? 1 : 0);
        mesh.draw();

        glm::mat4 lightModel{1.0F};
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3{0.18F});
        glUseProgram(lightProgram);
        tutorial::setMat4(lightProgram, "uModel", lightModel);
        tutorial::setMat4(lightProgram, "uView", view);
        tutorial::setMat4(lightProgram, "uProjection", projection);
        mesh.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
        if (maxFrames > 0 && ++frame >= maxFrames) glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    mesh.destroy();
    glDeleteProgram(objectProgram);
    glDeleteProgram(lightProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
