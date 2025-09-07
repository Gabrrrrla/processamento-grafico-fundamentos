/*
@author: Gabriela Bley Rodrigues
Comentário dos exercícios:
Para o octágono, pentágono, pac-man, fatia de pizza e a estrela, eu testei
com as seguintes configurações:

OCTÁGONO - mudei o N para 8, e no main glDrawArrays(GL_LINE_LOOP, 0, N); 

PENTÁGONO - mudei o N para 5, e no main glDrawArrays(GL_LINE_LOOP, 0, N); 

PAC-MAN - precisa remover uma fatia do círculo, fiz 30° a 330°. 
    N = 100
    const float start = M_PI / 6;   // 30 graus
    const float end   = 11*M_PI/6;  // 330 graus
    for (int i = 0; i <= N; i++) {
        float theta = start + (end-start) * i / N;
        vertices[(i+1)*3 + 0] = r * cos(theta);
        vertices[(i+1)*3 + 1] = r * sin(theta);
        vertices[(i+1)*3 + 2] = 0.0f;
    }
    No main, glDrawArrays(GL_TRIANGLE_FAN, 0, N+2);

FATIA DE PIZZA - eu só diminui a angulatura pra 0° a 60°

ESTRELA - é o código abaixo

*/

#include <iostream>
#include <string>
#include <assert.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std;


GLsizei g_numVertices = 0; // qtde de vértices a desenhar

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 void main()
 {
	 gl_Position = vec4(position.x, position.y, position.z, 1.0);
 }
 )";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
 #version 400
 uniform vec4 inputColor;
 out vec4 color;
 void main()
 {
	 color = inputColor;
 }
 )";


int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Criação da janela GLFW
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Estrela da Gabi", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Callback de teclado
    glfwSetKeyCallback(window, key_callback);

    // Inicialização do GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Informações do renderer
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version  = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Configuração da viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilação do shader
    GLuint shaderID = setupShader();

    // Configuração da geometria (estrela)
    GLuint VAO = setupGeometry();

    // Local da cor no fragment shader
    GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

    glUseProgram(shaderID);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Limpa a tela
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Configurações de desenho
        glLineWidth(3.0f);
        glBindVertexArray(VAO);
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f); // cor da estrela

        // Desenha a estrela
        glDrawArrays(GL_LINE_LOOP, 0, g_numVertices);

        // Troca os buffers
        glfwSwapBuffers(window);
    }

    // Limpeza de recursos
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}


// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}


int setupGeometry()
{
    const int spikes = 5;       // Quantidade de spikes da estrela
    const float r1 = 0.8f;      // Raio externo
    const float r2 = 0.4f;      // Raio interno
    const int N = spikes * 2;   // Alterna entre ponto externo e interno

    GLfloat vertices[N * 3];    // N vértices, cada um com x, y, z

    for (int i = 0; i < N; i++) {
        float angle = i * M_PI / spikes; // Divide o círculo em 10 partes (2 por ponta)
        float radius = (i % 2 == 0) ? r1 : r2; // Alterna entre externo e interno
        vertices[i * 3 + 0] = radius * cos(angle);
        vertices[i * 3 + 1] = radius * sin(angle);
        vertices[i * 3 + 2] = 0.0f;
    }

    g_numVertices = N; // Atualiza a quantidade global de vértices

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

