# 🎨 Aplicativo de Efeitos em Imagens e Vídeo – Grau B

Projeto desenvolvido para o Grau B da disciplina de Processamento Gráfico: Fundamentos - Unisinos

## 🧠 Contexto

Este aplicativo permite **carregar imagens** ou **usar a webcam** para aplicar filtros, operações matemáticas e stickers de forma interativa.  
O foco é explorar **técnicas de processamento digital de imagens** utilizando a biblioteca **OpenCV** e **Tkinter** para interface.

## 🚀 Funcionalidades

### 🖼️ Modo Foto
- Carrega uma imagem do disco (`.jpg`, `.png`, etc.)
- Aplica filtros com parâmetros fixos (ex: suavização, realce, detecção de bordas)
- Adiciona stickers clicando sobre a imagem
- Salva o resultado final
- Reseta todas as operações

### 📹 Modo Vídeo
- Captura vídeo em tempo real da webcam
- Aplica filtros em tempo real
- Permite capturar e salvar um frame
- Resetar o fluxo para o vídeo original

## 🎚️ Filtros Implementados

|     Categoria       | Filtro                        |           Descrição                       |
|---------------------|-------------------------------|-------------------------------------------|
| Suavização          | Box Blur                      | Faz a média dos pixels vizinhos.          |
| Suavização          | Gaussian Blur                 | Reduz ruído suavemente.                   |
| Realce              | Sharpen                       | Realça bordas e detalhes.                 |
| Realce              | Laplaciano                    | Detecta variações bruscas de intensidade. |
| Realce              | Sobel (X e Y)                 | Detecta bordas horizontais e verticais.   |
| Transformação       | Grayscale                     | Converte para tons de cinza.              |
| Transformação       | Negativo                      | Inverte as cores.                         |
| Operação matemática | Adição / Subtração / Blending | Combina duas imagens com pesos fixos.     |
| Efeito criativo     | Emboss                        | Cria relevo nas bordas.                   |

## 💫 Stickers

- Base mínima de **5 stickers** (PNG com transparência) na pasta `stickers/`
- Seleção e posicionamento com o mouse (modo foto)

## 🧩 Estrutura do Repositório

```
TrabalhoGB/
├── main.py
├── stickers/
│   ├── a.png
│   ├── b.png
│   ├── c.png
│   ├── d.png
│   └── e.png
├── README.md
├── requirements.txt
└── .gitignore
```

## ⚙️ Como Executar

1. **Clone o repositório:**
   ```bash
   git clone https://github.com/Gabrrrrla/processamento-grafico-fundamentos.git
   cd TrabalhoGB
   ```

2. **Instale as dependências:**
   ```bash
   pip install -r requirements.txt
   ```

3. **Execute a aplicação:**
   ```bash
   python main.py
   ```

4. **Interaja pela interface:**
   - Escolha entre *Modo Foto* e *Modo Vídeo*
   - Aplique filtros e stickers
   - Salve ou resete a imagem

## 🧠 Tecnologias Utilizadas

- **Python 3.10+**
- **OpenCV** – Processamento de imagens e vídeo
- **Tkinter** – Interface gráfica simples e integrada
- **NumPy** – Operações matriciais
- **Pillow (PIL)** – Manipulação de imagens e stickers

 
## 👩‍💻 Autoria

Desenvolvido por **Gabriela Bley Rodrigues**, conforme orientações do professor **Rossana Baptista Queiroz** –  
Disciplina **Processamento Gráfico: Fundamentos**, **Escola Politécnica**, **UNISINOS**.

