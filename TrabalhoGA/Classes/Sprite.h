#ifndef SPRITE_H
#define SPRITE_H

#include <glm/glm.hpp>
#include <glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include <SOIL2/SOIL2.h>


class Sprite {
public:
    glm::vec2 Position;
    glm::vec2 Size;
    GLuint VAO, VBO, EBO;
    GLuint textureID;

    // Variáveis para a animação
    int numFrames;      // Número total de quadros na animação
    int currentFrame;
    float frameTime;    // Tempo de exibição de cada quadro
    float elapsedTime; // Tempo decorrido para a animação
    
    // Dimensões do spritesheet
    float totalTextureWidth;
    float totalTextureHeight;
    int row;
    float frameWidth;
    float frameHeight;

    Sprite(glm::vec2 pos, glm::vec2 size, 
        const char* texturePath, int frames, 
        float timePerFrame, float texWidth, 
        float texHeight, int row = 0, 
        float frameWidth = 64.0f, float frameHeight = 64.0f);
    
    void UpdateAnimation(float deltaTime);
    void Draw(Shader& shader);
    bool CheckCollision(const Sprite& other);
    void ChangeAnimation(const char* newTexturePath, int newFrames, float newTimePerFrame, int newRow = 0, float newTotalTextureWidth = 64.0f , float newTotalTextureHeight = 64.0f, float newFrameWidth = 64.0f, float newFrameHeight = 64.0f);
};

#endif