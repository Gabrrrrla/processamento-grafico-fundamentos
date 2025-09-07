/*
@author: Gabriela Bley Rodrigues
Comentário do exercício:
Aqui, temos VAOs separados pro telhado e o retângulo da casa em si.
GL_TRIANGLE_STRIP para a base da casa
GL_TRIANGLES para o telhado
*/
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
using namespace std;

GLuint VAO_casa, VBO_casa, VAO_telhado, VBO_telhado;

GLsizei g_numVertices_casa = 0;
GLsizei g_numVertices_telhado = 0;

const GLuint WIDTH = 800, HEIGHT = 600;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

const GLchar* vertexShaderSource = R"(
#version 400
layout(location = 0) in vec3 position;
void main() { gl_Position = vec4(position, 1.0); }
)";

const GLchar* fragmentShaderSource = R"(
#version 400
uniform vec4 inputColor;
out vec4 color;
void main() { color = inputColor; }
)";

GLuint setupShader();
void setupGeometry();

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Casa OpenGL", nullptr, nullptr);
    if (!window) { cerr << "Falha ao criar janela\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cerr << "Falha GLAD\n"; return -1; }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();
    setupGeometry();

    GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");
    glUseProgram(shaderID);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.5f, 0.8f, 0.9f, 1.0f); // Fundo azul
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha a casa
        glBindVertexArray(VAO_casa);
        glUniform4f(colorLoc, 0.7f, 0.3f, 0.1f, 1.0f); // Marrom
        glDrawArrays(GL_TRIANGLE_STRIP, 0, g_numVertices_casa);

        // Desenha o telhado 
        glBindVertexArray(VAO_telhado);
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Vermelho
        glDrawArrays(GL_TRIANGLES, 0, g_numVertices_telhado);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO_casa);
    glDeleteBuffers(1, &VBO_casa);
    glDeleteVertexArrays(1, &VAO_telhado);
    glDeleteBuffers(1, &VBO_telhado);

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLuint setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void setupGeometry() {
    GLfloat verticesCasa[] = { // Retângulo da casa
        -0.5f,-0.5f,0.0f,
         0.5f,-0.5f,0.0f,
        -0.5f, 0.0f,0.0f,
         0.5f, 0.0f,0.0f
    };

    g_numVertices_casa = 4;
    glGenVertexArrays(1, &VAO_casa);
    glGenBuffers(1, &VBO_casa);
    glBindVertexArray(VAO_casa);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_casa);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCasa), verticesCasa, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    GLfloat telhadoVertices[] = { // Triângulo do telhado
        -0.6f, 0.0f, 0.0f,
         0.6f, 0.0f, 0.0f,
         0.0f, 0.5f, 0.0f
    };
    g_numVertices_telhado = 3;
    glGenVertexArrays(1, &VAO_telhado);
    glGenBuffers(1, &VBO_telhado);
    glBindVertexArray(VAO_telhado);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_telhado);
    glBufferData(GL_ARRAY_BUFFER, sizeof(telhadoVertices), telhadoVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
}
