#include <iostream>
#include <vector>
#include <glad.h>
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/Shader.h"
#include "Classes/Sprite.h"

// Dimensões da tela
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool isJumping;
bool isDucking = false;
bool isHurt;
float duckHeight = 64.0f;
float originalHeight;
float verticalVelocity;
float jumpForce; // Força inicial do pulo
std::vector<Sprite> obstacles;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, Sprite& elfa) {
    if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) && !isJumping) {
        isJumping = true;
        verticalVelocity = jumpForce;
        elfa.ChangeAnimation("../Textures/pular.png", 5, 0.1f, 3, 320.0f, 256.0f); 
    }
    if ((glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) 
        && !isJumping) {
        if (!isDucking) {
            isDucking = true;
            elfa.Size.y = duckHeight;  // abaixa a elfa
            elfa.Position.y -= (originalHeight - duckHeight); // ajusta para o chão
            elfa.ChangeAnimation("../Textures/abaixar.png", 3, 0.1f, 3, 192.0f, 256.0f, 64.0f, 64.0f); // exemplo
        }
    } else {
        // Voltar a posição normal se não estiver pressionando
        if (isDucking) {
            isDucking = false;
            elfa.Position.y += (originalHeight - duckHeight);
            elfa.Size.y = originalHeight;
            elfa.ChangeAnimation("../Textures/correr.png", 8, 0.1f, 3, 512.0f, 256.0f, 64.0f, 64.0f);
        }
    }
}

// Matriz de Projeção Ortográfica
glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Elfa Corredora", NULL, NULL);
    if (window == NULL) {
        std::cout << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Criação do Shader
    Shader mainShader("../Shaders/vertex.vs", "../Shaders/fragment.fs");
    mainShader.use();
    mainShader.setMat4("projection", projection);
    mainShader.setInt("ourTexture", 0); 
    
    Sprite background
    (
        glm::vec2(0.0f, 0.0f),
        glm::vec2(800.0f, 650.0f),
        "../Textures/background.jpg",
        1,      // linhas na spritesheet (1, já que é só uma imagem)
        1.0f,   // velocidade de animação
        800.0f, // largura total da imagem
        650.0f, // altura total da imagem
        0,      // offset X da primeira frame
        800,    // largura do frame
        650     // altura do frame
    );

    // Criação da elfa (uma única variável)
    Sprite elfa
    (
        glm::vec2(0.0f, 150.0f), //altura do chao do background 
        glm::vec2(64.0f, 64.0f), 
        "../Textures/correr.png", 
        8, 
        0.1f, 
        512.0f, 
        256.0f,
        3,
        64,
        64
    );

    originalHeight = elfa.Size.y;

    // Obstáculos
    obstacles.push_back(Sprite
        (
            glm::vec2(600.0f, 150.0f), 
            glm::vec2(64.0f, 40.0f), 
            "../Textures/tronco.png", 
            1, 1.0f, 
            64.0f, 40.0f,
            0, 64, 40
        )
    );

    float movementSpeed = 200.0f; // velocidade em pixels/segundo
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    // As variáveis de pulo foram movidas para o escopo global ou do main para evitar redefinição
    verticalVelocity = 0.0f;
    jumpForce = 500.0f; // Força inicial do pulo
    float gravity = -980.0f; // Gravidade
    isJumping = false;

    // Loop de renderização
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime(); // Adicionado cast para float
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float obstacleSpeed = 200.0f;

        elfa.UpdateAnimation(deltaTime);
        if(isHurt){
            // Espera a animação "machucada" terminar
            if (elfa.currentFrame == elfa.numFrames - 1) {
                elfa.Position = glm::vec2(0.0f, 150.0f);
                isJumping = false;
                verticalVelocity = 0.0f;

                // Resetar obstáculos
                for (auto& o : obstacles) o.Position.x = 600.0f;

                elfa.ChangeAnimation("../Textures/correr.png", 8, 0.1f, 3, 512.0f, 256.0f, 64.0f, 64.0f);

                isHurt = false;
            }
        }
        else{
            processInput(window, elfa); 

            // Movimento automático para a direita
            elfa.Position.x += movementSpeed * deltaTime;
            if (elfa.Position.x > SCR_WIDTH - elfa.Size.x) {
                elfa.Position.x = 0.0f;
            }

            // Lógica do pulo
            if (isJumping) {
                verticalVelocity += gravity * deltaTime;
                elfa.Position.y += verticalVelocity * deltaTime;

                // Se a elfa voltou ao chão
                if (elfa.Position.y <= 150.0f) {
                    elfa.Position.y = 150.0f;
                    isJumping = false;
                    verticalVelocity = 0.0f;
                }
                if(!isDucking){
                    elfa.Size.y = originalHeight;
                    elfa.ChangeAnimation("../Textures/correr.png", 8, 0.1f, 3, 512.0f, 256.0f);
                }

            }
            
            // Atualizar obstáculos
            for (auto& obs : obstacles) {
                obs.Position.x -= obstacleSpeed * deltaTime;

                if (obs.Position.x + obs.Size.x < 0) {
                    // Nova posição aleatória no eixo X (um pouco depois da tela)
                    float randomOffsetX = 200.0f + (rand() % 400); 
                    obs.Position.x = SCR_WIDTH + randomOffsetX;

                    // Nova altura aleatória (escolhendo entre alguns níveis)
                    float possibleHeights[] = {150.0f, 210.0f, 220.0f};
                    int randomIndex = rand() % 3;
                    obs.Position.y = possibleHeights[randomIndex];
                }
            }


            // Checar colisão
            for (auto& obs : obstacles) {
                if (elfa.CheckCollision(obs)) {
                    isHurt = true;
                    // Troca animação para machucada
                    elfa.ChangeAnimation("../Textures/machucada.png", 6, 0.1f, 0, 384.0f, 64.0f);
                    // A elfa morreu! Reinicia posição e pulo
                    // elfa.Position = glm::vec2(150.0f, 150.0f);
                    // isJumping = false;
                    // verticalVelocity = 0.0f;

                    // // Reinicia obstáculos
                    // for (auto& o : obstacles) o.Position.x = 600.0f;

                    break; // não precisa checar outros obstáculos
                }
            }
        }  
    
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha o background
        background.Draw(mainShader);

        // Desenhar a elfa
        elfa.Draw(mainShader);

        // Desenhar obstáculos
        for (auto& obs : obstacles) {
            obs.Draw(mainShader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}