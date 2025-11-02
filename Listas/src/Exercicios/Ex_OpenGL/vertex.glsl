#version 330 core

layout (location = 0) in vec2 aPos;

// Matriz de transformação opcional (identidade se não usada)
uniform mat4 transform;

void main()
{
    // Aplica a transformação (ou identidade se nada for enviado)
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
}
