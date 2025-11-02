#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------
// Função para ler arquivos de shader
// ---------------------------------------------------------
std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ---------------------------------------------------------
// Função para compilar shader e criar programa
// ---------------------------------------------------------
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vCode = loadShaderSource(vertexPath);
    std::string fCode = loadShaderSource(fragmentPath);
    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, info);
        std::cerr << "Erro ao compilar Vertex Shader:\n" << info << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, info);
        std::cerr << "Erro ao compilar Fragment Shader:\n" << info << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, info);
        std::cerr << "Erro ao linkar programa:\n" << info << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// ---------------------------------------------------------
// Exercício 1: Função createTriangle
// ---------------------------------------------------------
GLuint createTriangle(float x0, float y0, float x1, float y1, float x2, float y2) {
    float vertices[] = {
        x0, y0,
        x1, y1,
        x2, y2
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// ---------------------------------------------------------
// Struck do exercício 3
// ---------------------------------------------------------
struct Triangle {
    glm::vec2 position;
    glm::vec3 color;
};

std::vector<Triangle> dynamicTriangles;
GLuint standardVAO;
GLuint shaderProgram;

// ---------------------------------------------------------
// Exercício 3
// ---------------------------------------------------------
void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        int width, height;
        glfwGetCursorPos(window, &xpos, &ypos);
        glfwGetWindowSize(window, &width, &height);

        // Converte de pixel → NDC
        float x = (xpos / width) * 2.0f - 1.0f;
        float y = 1.0f - (ypos / height) * 2.0f;

        Triangle t;
        t.position = glm::vec2(x, y);
        t.color = glm::vec3(
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f,
            (rand() % 100) / 100.0f
        );

        dynamicTriangles.push_back(t);
    }
}

// ---------------------------------------------------------
// main
// ---------------------------------------------------------
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Exercicios OpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // --- Exercício 2: cinco triângulos fixos ---
    std::vector<GLuint> staticTriangles;
    staticTriangles.push_back(createTriangle(-0.5f, -0.5f, -0.3f, -0.5f, -0.4f, -0.3f));
    staticTriangles.push_back(createTriangle(0.2f, 0.1f, 0.4f, 0.1f, 0.3f, 0.3f));
    staticTriangles.push_back(createTriangle(-0.2f, 0.3f, 0.0f, 0.3f, -0.1f, 0.5f));
    staticTriangles.push_back(createTriangle(0.1f, -0.6f, 0.3f, -0.6f, 0.2f, -0.4f));
    staticTriangles.push_back(createTriangle(-0.8f, 0.0f, -0.6f, 0.0f, -0.7f, 0.2f));

    std::vector<glm::vec3> colors = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},
        {1.0f, 0.0f, 1.0f}
    };

    // --- Exercício 3 ---
    glfwSetMouseButtonCallback(window, mouse_callback);
    standardVAO = createTriangle(-0.1f, -0.1f, 0.1f, -0.1f, 0.0f, 0.1f);

    // --- Loop principal ---
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Identidade para triângulos fixos (sem transformação)
        glm::mat4 identity = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &identity[0][0]);

        // Exercício 2: desenhar os triângulos fixos
        for (size_t i = 0; i < staticTriangles.size(); ++i) {
            glm::vec3 color = colors[i % colors.size()];
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
            glBindVertexArray(staticTriangles[i]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // Exercício 3: triângulos dinâmicos (clicados)
        for (auto& t : dynamicTriangles) {
            glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(t.position, 0.0f));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);
            glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &t.color[0]);

            glBindVertexArray(standardVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
