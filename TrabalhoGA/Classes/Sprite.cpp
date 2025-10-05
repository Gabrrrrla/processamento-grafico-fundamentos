#include "Sprite.h"
#include <iostream>

Sprite::Sprite(glm::vec2 pos, glm::vec2 size, const char* texturePath, int frames, float timePerFrame, float texWidth, float texHeight, int row, float frameW, float frameH) : Position(pos), Size(size), numFrames(frames), frameTime(timePerFrame), currentFrame(0), elapsedTime(0.0f), totalTextureWidth(texWidth), totalTextureHeight(texHeight), row(row), frameWidth(frameW), frameHeight(frameH) {
    std::string currentAnimationName;

    // Carregar a textura
    textureID = SOIL_load_OGL_texture(texturePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (textureID == 0) {
        std::cout << "ERRO: Falha ao carregar a textura: " << texturePath << std::endl;
    }
    
    // Vértices do polígono (quadrilátero)
    GLfloat vertices[] = {
        // Posição        // Coordenadas de Textura (inicializadas como 0)
        0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,  0.0f, 0.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        0, 3, 1
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Sprite::UpdateAnimation(float deltaTime) {
    elapsedTime += deltaTime;
    if (elapsedTime >= frameTime) {
        currentFrame = (currentFrame + 1) % numFrames;
        elapsedTime = 0.0f;
    }
}

void Sprite::ChangeAnimation(const char* newTexturePath, int newFrames, float newTimePerFrame, int newRow, float newTotalTextureWidth, float newTotalTextureHeight, float newFrameWidth, float newFrameHeight) 
{
    // Carrega a nova textura e atualiza as variáveis de animação
    textureID = SOIL_load_OGL_texture(newTexturePath, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (textureID == 0) {
        std::cout << "ERRO: Falha ao carregar a nova textura: " << newTexturePath << std::endl;
    }
    numFrames = newFrames;
    frameTime = newTimePerFrame;
    currentFrame = 0;
    elapsedTime = 0.0f;
    row = newRow; 
    frameWidth = newFrameWidth; 
    frameHeight = newFrameHeight;
    totalTextureHeight = newTotalTextureHeight;
    totalTextureWidth = newTotalTextureWidth;
}

bool Sprite::CheckCollision(const Sprite& other) {
    bool collisionX = Position.x + Size.x >= other.Position.x &&
                      other.Position.x + other.Size.x >= Position.x;
    bool collisionY = Position.y + Size.y >= other.Position.y &&
                      other.Position.y + other.Size.y >= Position.y;
    return collisionX && collisionY;
}


void Sprite::Draw(Shader& shader) {
    shader.use();
    
    // Matriz de Modelo (posição e escala)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(Position.x, Position.y, 0.0f));
    model = glm::scale(model, glm::vec3(Size.x, Size.y, 1.0f));
    shader.setMat4("model", model);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    
    // Calcula UVs com base nos parâmetros configuráveis 
    float u = (float)currentFrame * (frameWidth / totalTextureWidth); 
    float v = 1.0f - ((float)row + 1.0f) * (frameHeight / totalTextureHeight);
    //if()
    printf("height - %f %f \n",frameHeight, totalTextureHeight);
    printf("width %f %f \n",frameWidth, totalTextureWidth);

    GLfloat vertices[] = {
        // Posição           // Coordenadas de Textura
        0.0f, 1.0f, 0.0f,    u,                      v + frameHeight / totalTextureHeight,
        1.0f, 0.0f, 0.0f,    u + frameWidth / totalTextureWidth, v,
        0.0f, 0.0f, 0.0f,    u,                      v,
        1.0f, 1.0f, 0.0f,    u + frameWidth / totalTextureWidth, v + frameHeight / totalTextureHeight
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}