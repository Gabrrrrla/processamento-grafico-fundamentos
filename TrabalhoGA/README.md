# Elfa Corredora ✨

**Elfa Corredora** é um jogo 2D simples desenvolvido em C++ usando OpenGL, GLFW e GLM. O jogador controla uma elfa que corre automaticamente, podendo pular e abaixar para evitar obstáculos e coletar cristais.

---

## Funcionalidades

- Movimentação automática da elfa da esquerda para a direita.
- Pular e abaixar para evitar obstáculos.
- Obstáculos (troncos) que causam dano ao colidir.
- Cristais que podem ser coletados.
- Evita sobreposição horizontal de obstáculos e cristais.
- Animações para correr, pular, abaixar e quando a elfa é machucada.
- Fundo estático ou scrolling opcional.
- Sistema de colisão simples baseado em sprites.

---

## Dependências

O projeto utiliza as seguintes bibliotecas:

- **GLFW** – Para criação da janela e gerenciamento de input.  
  [GLFW Official](https://www.glfw.org/)
- **GLAD** – Para carregar funções OpenGL.  
  [GLAD Official](https://glad.dav1d.de/)
- **GLM** – Para matemática de vetores e matrizes (projeção ortográfica, transformações).  
  [GLM GitHub](https://github.com/g-truc/glm)
- **C++17** ou superior.
- Sistema operacional compatível com OpenGL (Windows, Linux, macOS).

---

## Estrutura do projeto

```
TrabalhoGA/
│
├─ Classes/
│  ├─ Shader.h/.cpp         # Classe para shaders
│  └─ Sprite.h/.cpp         # Classe para sprites, animações e colisão
│
├─ Shaders/
│  ├─ vertex.vs             # Vertex shader
│  └─ fragment.fs           # Fragment shader
│
├─ Textures/
│  ├─ correr.png
│  ├─ pular.png
│  ├─ abaixar.png
│  ├─ machucada.png
│  ├─ tronco.png
│  ├─ cristal.png
│  └─ background.jpg
│
├─ main.cpp                 # Código principal do jogo
└─ README.md
```

---

## Como compilar

1. Clone o repositório:

```bash
git clone https://github.com/seu-usuario/ElfaCorredora.git
cd ElfaCorredora
```

2. Certifique-se de ter as dependências instaladas (GLFW, GLAD, GLM).

3. Compile usando `g++` (exemplo no Linux/Windows com MinGW):

```bash
g++ main.cpp Classes/*.cpp -o ElfaCorredora -lglfw3 -lGL -ldl -lX11 -pthread
```

> Ajuste os parâmetros de link de acordo com seu sistema operacional.

4. Execute o jogo:

```bash
./ElfaCorredora
```

---

## Controles

- **Espaço / Seta para cima:** Pular  
- **S / Seta para baixo:** Abaixar

---

## Próximos recursos 🔮

- Sistema de pontuação baseado nos cristais coletados.  
- Background scrolling contínuo.  
- Vários níveis de dificuldade e velocidade crescente.  
- Sons para coleta de cristais e colisão com obstáculos.  

---

## Licença 🪪

Este projeto é **open source** e pode ser usado e modificado livremente.
