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
float duckHeight = 40.0f;
float originalHeight;
float verticalVelocity;
float jumpForce; 

std::vector<Sprite> obstacles;
std::vector<Sprite> crystals;

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
            elfa.Size.y = duckHeight;  // Abaixa a elfa
            elfa.Position.y = 150.0f;
            elfa.ChangeAnimation("../Textures/abaixar.png", 3, 0.1f, 3, 192.0f, 256.0f, 64.0f, 64.0f); 
        }
    } else {
        if (isDucking) {
            isDucking = false;
            elfa.Position.y += (originalHeight - duckHeight);
            elfa.Size.y = originalHeight;
            elfa.Position.y = 150.0f;  
            elfa.ChangeAnimation("../Textures/correr.png", 8, 0.1f, 3, 512.0f, 256.0f, 64.0f, 64.0f);
        }
    }
}

// Matriz de Projeção Ortográfica
glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, -1.0f, 1.0f);


int main() {
    float movementSpeed = 200.0f; 
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    float gravity = -980.0f; 
    float backgroundX = 0.0f; 
    float backgroundSpeed = 100.0f; 
    jumpForce = 500.0f;
    isJumping = false; 
    verticalVelocity = 0.0f;
    
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

    /* 
    Ativa transparência para sprites.
    Cria e usa o shader principal.
    Passa a matriz de projeção para o shader.
    Diz qual unidade de textura o shader deve usar.
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader mainShader("../Shaders/vertex.vs", "../Shaders/fragment.fs");
    mainShader.use();
    mainShader.setMat4("projection", projection);
    mainShader.setInt("ourTexture", 0); 
    
    // Criação do background
    Sprite background
    (
        glm::vec2(0.0f, 0.0f),
        glm::vec2(800.0f, 650.0f),
        "../Textures/background.jpg",
        1,      
        1.0f,   
        800.0f, 
        650.0f, 
        0,      
        800,    
        650     
    );

    // Criação da elfa 
    Sprite elfa
    (
        glm::vec2(0.0f, 150.0f), 
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

    // Criação dos obstáculos "ruins" (troncos)
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

    // Criação dos obstáculos "bons" (cristais)
    crystals.push_back(Sprite
        (
            glm::vec2(200.0f, 150.0f), 
            glm::vec2(64.0f, 40.0f), 
            "../Textures/cristal.png", 
            1, 1.0f, 
            64.0f, 40.0f,
            0, 64, 40
        )
    );

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float)glfwGetTime();
        float obstacleSpeed = 250.0f;
        float minDistance = 100.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        backgroundX -= backgroundSpeed * deltaTime;

        // Chama a função que faz a animação da elfa avançar quadro a quadro, usando o tempo que passou
        elfa.UpdateAnimation(deltaTime);

        if(isHurt){
            // Espera a animação "machucada" terminar
            if (elfa.currentFrame == elfa.numFrames - 1) {
                elfa.Position = glm::vec2(0.0f, 150.0f);
                isJumping = false;
                verticalVelocity = 0.0f;

                // Resetar obstáculos
                for (auto& o : obstacles) o.Position.x = 600.0f;
                for (auto& c : crystals) c.Position.x = 600.0f; 

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
                        float newX;
                        bool overlap;

                        // Tenta gerar uma posição nova sem sobreposição
                        do {
                            newX = SCR_WIDTH + 200.0f + static_cast<float>(rand() % 400);
                            overlap = false;

                            // Checa contra outros obstáculos
                            for (auto& other : obstacles) {
                                if (&other != &obs && fabs(newX - other.Position.x) < 100.0f) {
                                    overlap = true;
                                    break;
                                }
                            }

                        // Checa contra cristais
                        for (auto& c : crystals) {
                            if (fabs(newX - c.Position.x) < 100.0f) {
                                overlap = true;
                                break;
                            }
                        }
                        } while (overlap);

                    obs.Position.x = newX;

                    // Altura aleatória
                    float possibleHeights[] = {150.0f, 195.0f, 210.0f};
                    int randomIndex = rand() % 3;
                    obs.Position.y = possibleHeights[randomIndex];
                }   
            }

            // Atualizar cristais
            for (auto& c : crystals) {
                c.Position.x -= obstacleSpeed * deltaTime;

                if (c.Position.x + c.Size.x < 0) {
                    float newX;
                    bool overlap;

                    // Tenta gerar uma posição nova sem sobreposição
                    do {
                        newX = SCR_WIDTH + 200.0f + static_cast<float>(rand() % 400);
                        overlap = false;

                        // Checa contra outros cristais
                        for (auto& other : crystals) {
                            if (&other != &c && fabs(newX - other.Position.x) < 100.0f) {
                                overlap = true;
                                break;
                            }
                        }

                        // Checa contra obstáculos
                        for (auto& obs : obstacles) {
                            if (fabs(newX - obs.Position.x) < 100.0f) {
                                overlap = true;
                                break;
                            }
                        }
                    } while (overlap);

                    c.Position.x = newX;

                    // Altura aleatória
                    float possibleHeights[] = {150.0f, 210.0f, 220.0f};
                    int randomIndex = rand() % 3;
                    c.Position.y = possibleHeights[randomIndex];
                }
            }

            // Checar colisão
            for (auto& obs : obstacles) {
                if (elfa.CheckCollision(obs)) {
                    isHurt = true;
                    elfa.ChangeAnimation("../Textures/machucada.png", 6, 0.1f, 0, 384.0f, 64.0f);
                    break; 
                }
            }

            // Checar colisão com os cristais
            for (auto& c : crystals) {
                if (elfa.CheckCollision(c)) {
                    c.Position.x = SCR_WIDTH + 200 + (rand() % 400);
                }
            }

        // Se o primeiro background saiu da tela, reinicia a posição (loop horizontal)
        if (backgroundX <= -background.Size.x) {
            backgroundX = 0.0f;
        }

    }  
    
        // Parte da renderização

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Primeiro background
        background.Position.x = backgroundX;
        background.Draw(mainShader);

        // Segundo background, logo após o primeiro
        background.Position.x = backgroundX + background.Size.x;
        background.Draw(mainShader);

        // Desenhar a elfa
        elfa.Draw(mainShader);

        // Desenhar troncos
        for (auto& obs : obstacles) {
            obs.Draw(mainShader);
        }

        // Desenhar os cristais
        for (auto& c : crystals) {
            c.Draw(mainShader);
        }

        /*
        glfwSwapBuffers troca o buffer de vídeo e
        glfwPollEvents lê os eventos da janela (teclado, mouse, fechar janela).
        */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}