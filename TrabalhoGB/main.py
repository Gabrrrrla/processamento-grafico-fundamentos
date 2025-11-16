"""
Gabriela Bley Rodrigues

p rodar: python main.py
"""

import os
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from PIL import Image, ImageTk
import cv2
import numpy as np

# ------------------- Config -------------------
STICKERS_DIR = 'stickers'
WINDOW_TITLE = 'Editor de imagens e vídeo'
CAM_WIDTH = 640
CAM_HEIGHT = 480

# ------------------- Filtros com valores fixos -------------------
FILTERS = [
    'none',
    'gaussian',
    'box',
    'median',
    'bilateral',
    'sharpen',
    'unsharp',
    'laplacian',
    'emboss',
    'canny',
    'hist_eq'
]

"""
    gaussian: 5x5, sigma=1.0
    box: média 5x5
    median: k=5
"""


FILTER_INFO = {
    'none': 'Sem filtro.',
    'gaussian': 'Suavização Gaussiana (5x5, sigma=1.0). Remove ruído suave.',
    'box': 'Blur Box (média 5x5). Suavização simples.',
    'median': 'MedianBlur (k=5). Bom para ruído sal-e-pimenta.',
    'bilateral': 'Filtro bilateral: suaviza mantendo bordas.',
    'sharpen': 'Realça bordas (filtro de nitidez).',
    'unsharp': 'Unsharp mask (realce via subtração de blur).',
    'laplacian': 'Laplaciano: detecção de bordas (segunda derivada).',
    'emboss': 'Emboss: relevo, efeito artístico.',
    'canny': 'Detecção de bordas Canny. Retorna linhas das bordas.',
    'hist_eq': 'Equalização de histograma (grayscale).'
}

# ------------------- Utils: filtros -------------------

def apply_filter_cv(img_bgr, name):
    """Aplica o filtro e sempre retorna uma imagem BGR (3 canais)."""
    if img_bgr is None:
        return None
    img = img_bgr.copy()
    if name == 'none':
        return img
    if name == 'gaussian':
        return cv2.GaussianBlur(img, (5,5), 1.0)
    if name == 'box':
        return cv2.blur(img, (5,5))
    if name == 'median':
        return cv2.medianBlur(img, 5)
    if name == 'bilateral':
        return cv2.bilateralFilter(img, 9, 75, 75)
    if name == 'sharpen':
        kernel = np.array([[0,-1,0],[-1,5,-1],[0,-1,0]], dtype=np.float32)
        return cv2.filter2D(img, -1, kernel)
    if name == 'unsharp':
        blurred = cv2.GaussianBlur(img, (9,9), 10.0)
        return cv2.addWeighted(img, 1.5, blurred, -0.5, 0)
    if name == 'laplacian':
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        lap = cv2.Laplacian(gray, cv2.CV_64F, ksize=3)
        lap = cv2.convertScaleAbs(lap)
        return cv2.cvtColor(lap, cv2.COLOR_GRAY2BGR)
    if name == 'emboss':
        kernel = np.array([[-2,-1,0],[-1,1,1],[0,1,2]], dtype=np.float32)
        emb = cv2.filter2D(img, -1, kernel) + 128
        return cv2.convertScaleAbs(emb)
    if name == 'canny':
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray, 50, 150)
        return cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
    if name == 'hist_eq':
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        eq = cv2.equalizeHist(gray)
        return cv2.cvtColor(eq, cv2.COLOR_GRAY2BGR)
    return img

# ------------------- Utils: stickers -------------------

def load_stickers(dirpath, size=(400, 400)):
    """
    Carrega os stickers PNG com alfa. Retorna dict: chave_limpa -> {'img':np.array(rgba),'file':filename, 'orig_size':(w,h)}
    Mantém imagens no tamanho original (ou redimensiona inicialmente para size se muito grande).
    """
    stickers = {}
    if not os.path.exists(dirpath):
        print(f"Pasta de stickers '{dirpath}' não encontrada.")
        return stickers

    for fname in sorted(os.listdir(dirpath)):
        if fname.lower().endswith('.png'):
            path = os.path.join(dirpath, fname)
            im = cv2.imread(path, cv2.IMREAD_UNCHANGED)  # BGRA
            if im is None:
                print(f"Não foi possível carregar: {fname}")
                continue

            h, w = im.shape[:2]
            # limitar tamanho inicial para evitar stickers gigantes
            max_side = max(w, h)
            if max_side > size[0]:
                scale = size[0] / max_side
                new_w = int(w * scale)
                new_h = int(h * scale)
                im = cv2.resize(im, (new_w, new_h), interpolation=cv2.INTER_AREA)
                h, w = im.shape[:2]

            clean = os.path.splitext(fname)[0]  # remove extensão
            stickers[clean] = {'img': im, 'file': fname, 'orig_size': (w, h)}
    return stickers


def overlay_alpha(bg, fg_rgba, x, y):
    """
    Sobrepõe fg_rgba (BGRA ou BGR) sobre bg (BGR) na posição x,y (coordenadas da imagem).
    Retorna nova imagem (cópia).
    """
    out = bg.copy()
    bh, bw = out.shape[:2]
    fh, fw = fg_rgba.shape[:2]
    if x >= bw or y >= bh:
        return out
    w = min(fw, bw - x)
    h = min(fh, bh - y)
    if w <= 0 or h <= 0:
        return out
    fg_crop = fg_rgba[0:h, 0:w]
    if fg_crop.shape[2] == 4:
        alpha = fg_crop[:, :, 3].astype(np.float32) / 255.0
        for c in range(3):
            out[y:y+h, x:x+w, c] = (alpha * fg_crop[:, :, c] + (1-alpha) * out[y:y+h, x:x+w, c])
    else:
        out[y:y+h, x:x+w] = fg_crop[:, :, :3]
    return out

# ------------------- Utils: conversão -------------------

def cv2_to_pil(img_bgr):
    img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)
    return Image.fromarray(img_rgb)


def pil_to_cv2(pil_img):
    arr = np.array(pil_img)
    if arr.ndim == 2:
        return cv2.cvtColor(arr, cv2.COLOR_GRAY2BGR)
    return cv2.cvtColor(arr, cv2.COLOR_RGB2BGR)

# ------------------- Interface gráfica -------------------

class EditorApp:
    def __init__(self, root):
        self.root = root
        self.root.title(WINDOW_TITLE)

        self.mode = 'photo'
        self.cap = None
        self.cam_thread = None
        self.cam_running = False

        self.original = None  # imagem original de background (BGR)
        self.current = None   # imagem atual do background com filtros etc (BGR)
        self.selected_filter = tk.StringVar(value='none')

        # stickers
        self.stickers = load_stickers(STICKERS_DIR)
        self.sticker_labels = ['Nenhum'] + list(self.stickers.keys())
        self.selected_sticker_name = tk.StringVar(value='Nenhum')

        # segunda imagem
        self.second_image = None

        # opções de filtro
        self.filter_overwrite = tk.BooleanVar(value=True)  # True = aplica sempre sobre original

        # atributos de exibição (para mapear clique)
        self.display_scale = 1.0
        self.display_offset = (0, 0)
        self.display_size = (0, 0)  # (nw, nh)

        # sticker em movimento/resize
        self.moving = False
        self.moving_mode = None  # 'move' ou 'resize'
        self.moving_sticker_key = None
        self.moving_sticker_img = None  # RGBA numpy array
        self.moving_sticker_pos = (0, 0)  # coordenadas na imagem (não no canvas)
        self.moving_sticker_size = (0, 0)  # w,h em pixels (no espaço da imagem)
        self.temp_image = None  # cópia de current usada durante preview

        # UI
        self._build_ui()
        self._update_sticker_menu()
        self._refresh_canvas()

    def _build_ui(self):
        left = tk.Frame(self.root)
        left.pack(side=tk.LEFT, padx=5, pady=5)

        self.canvas = tk.Canvas(left, width=CAM_WIDTH, height=CAM_HEIGHT, bg='black')
        self.canvas.pack()
        self.canvas.bind('<Button-1>', self.on_canvas_click)
        self.canvas.bind('<B1-Motion>', self.on_canvas_drag)
        self.canvas.bind('<ButtonRelease-1>', self.on_canvas_release)

        right = tk.Frame(self.root)
        right.pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=5)

        # modo
        mode_frame = tk.LabelFrame(right, text='Modo')
        mode_frame.pack(fill=tk.X, pady=4)
        tk.Button(mode_frame, text='Modo Foto', command=self.set_mode_photo).pack(side=tk.LEFT, padx=4, pady=4)
        tk.Button(mode_frame, text='Modo Vídeo', command=self.set_mode_video).pack(side=tk.LEFT, padx=4, pady=4)

        # carrega, salva e reseta
        io_frame = tk.LabelFrame(right, text='Arquivo')
        io_frame.pack(fill=tk.X, pady=4)
        tk.Button(io_frame, text='Carregar imagem', command=self.load_image).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(io_frame, text='Salvar imagem', command=self.save_image).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(io_frame, text='Resetar', command=self.reset_image).pack(fill=tk.X, padx=4, pady=2)

        # filtros
        filt_frame = tk.LabelFrame(right, text='Filtros')
        filt_frame.pack(fill=tk.X, pady=4)
        self.filter_combo = ttk.Combobox(filt_frame, values=FILTERS, textvariable=self.selected_filter, state='readonly')
        self.filter_combo.pack(fill=tk.X, padx=4, pady=2)
        self.filter_combo.bind('<<ComboboxSelected>>', self.on_filter_selected)
        tk.Button(filt_frame, text='Aplicar filtro', command=self.apply_filter_button).pack(fill=tk.X, padx=4, pady=2)
        tk.Checkbutton(filt_frame, text='Aplicar sempre sobre ORIGINAL (não empilhar)', variable=self.filter_overwrite).pack(anchor=tk.W, padx=4)
        self.filter_desc_label = tk.Label(filt_frame, text='Descrição: -', wraplength=220, justify=tk.LEFT)
        self.filter_desc_label.pack(fill=tk.X, padx=4, pady=2)

        # operações de aritmética
        arith_frame = tk.LabelFrame(right, text='Operações aritméticas (2 imagens)')
        arith_frame.pack(fill=tk.X, pady=4)
        tk.Button(arith_frame, text='Carregar 2ª imagem', command=self.load_second_image).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(arith_frame, text='Add', command=lambda: self.apply_arith('add')).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(arith_frame, text='Sub', command=lambda: self.apply_arith('subtract')).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(arith_frame, text='Blend (0.5/0.5)', command=lambda: self.apply_arith('blend')).pack(fill=tk.X, padx=4, pady=2)

        # stickers
        sticker_frame = tk.LabelFrame(right, text='Stickers (modo foto)')
        sticker_frame.pack(fill=tk.X, pady=4)
        self.sticker_combo = ttk.Combobox(sticker_frame, values=self.sticker_labels, textvariable=self.selected_sticker_name, state='readonly')
        self.sticker_combo.pack(fill=tk.X, padx=4, pady=2)
        tk.Label(sticker_frame, text='Clique na imagem para posicionar. Arraste para mover. Arraste no canto inferior-direito do sticker para redimensionar.').pack()

        # ajuda e sair
        misc_frame = tk.Frame(right)
        misc_frame.pack(fill=tk.X, pady=8)
        tk.Button(misc_frame, text='Ajuda', command=self.show_help).pack(fill=tk.X, padx=4, pady=2)
        tk.Button(misc_frame, text='Sair', command=self.on_close).pack(fill=tk.X, padx=4, pady=2)

        self._photoimage = None

    # --------- Seleção de modo ---------
    def set_mode_photo(self):
        if self.cam_running:
            self.stop_camera()
        self.mode = 'photo'
        self._refresh_canvas()

    def set_mode_video(self):
        self.mode = 'video'
        self.start_camera()

    # --------- E/S ---------
    def load_image(self):
        path = filedialog.askopenfilename(title='Selecionar imagem', filetypes=[('Image','*.png;*.jpg;*.jpeg;*.bmp;*.tiff')])
        if not path:
            return
        img = cv2.imread(path)
        if img is None:
            messagebox.showerror('Erro', 'Não foi possível abrir a imagem')
            return
        img = img.copy()
        self.original = img.copy()
        self.current = img.copy()
        self.mode = 'photo'
        self._refresh_canvas()

    def save_image(self):
        if self.current is None:
            messagebox.showinfo('Salvar', 'Nenhuma imagem para salvar')
            return
        path = filedialog.asksaveasfilename(defaultextension='.jpg', filetypes=[('JPEG','*.jpg'),('PNG','*.png')])
        if not path:
            return
        cv2.imwrite(path, self.current)
        messagebox.showinfo('Salvar', f'Imagem salva em {path}')

    def reset_image(self):
        if self.original is not None:
            self.current = self.original.copy()
            self._refresh_canvas()
        else:
            messagebox.showinfo('Reset', 'Nenhuma imagem original carregada')

    # --------- Filtros e aritmética ---------
    def on_filter_selected(self, event=None):
        name = self.selected_filter.get()
        desc = FILTER_INFO.get(name, 'Sem descrição')
        self.filter_desc_label.config(text=f'Descrição: {desc}')

    def apply_filter_button(self):
        if self.current is None and not (self.mode == 'video' and self.cam_running):
            messagebox.showinfo('Filtro', 'Nenhuma imagem/stream ativo')
            return
        name = self.selected_filter.get()
        # Aplica conforme opção overwrite
        if self.mode == 'video' and self.cam_running:
            messagebox.showinfo('Filtro', f'Filtro {name} aplicado ao stream (valores fixos).')
            return
        base = self.original if self.filter_overwrite.get() and self.original is not None else self.current
        if base is None:
            messagebox.showinfo('Filtro', 'Nenhuma imagem base para aplicar.')
            return
        self.current = apply_filter_cv(base.copy(), name)
        self._refresh_canvas()

    def load_second_image(self):
        path = filedialog.askopenfilename(title='Selecionar 2ª imagem', filetypes=[('Image','*.png;*.jpg;*.jpeg;*.bmp;*.tiff')])
        if not path:
            return
        img = cv2.imread(path)
        if img is None:
            messagebox.showerror('Erro', 'Não foi possível abrir a imagem')
            return
        self.second_image = img
        messagebox.showinfo('2ª imagem', '2ª imagem carregada com sucesso')

    def apply_arith(self, op):
        if self.current is None:
            messagebox.showinfo('Operação', 'Nenhuma imagem atual')
            return
        if self.second_image is None:
            messagebox.showinfo('Operação', 'Carregue a 2ª imagem primeiro')
            return
        img1 = self.current
        img2 = cv2.resize(self.second_image, (img1.shape[1], img1.shape[0]))
        if op == 'add':
            self.current = cv2.add(img1, img2)
        elif op == 'subtract':
            self.current = cv2.subtract(img1, img2)
        elif op == 'blend':
            self.current = cv2.addWeighted(img1, 0.5, img2, 0.5, 0)
        self._refresh_canvas()

    # --------- Stickers: menu e utilidades ---------
    def _update_sticker_menu(self):
        self.sticker_labels = ['Nenhum'] + list(self.stickers.keys())
        self.sticker_combo['values'] = self.sticker_labels
        self.sticker_combo.current(0)
        self.selected_sticker_name.set('Nenhum')

    # --------- Canvas mouse handlers (mapeamento coordenadas) ---------
    def image_coords_from_canvas(self, cx, cy):
        """Mapeia coordenadas do canvas para coordenadas da imagem atual exibida."""
        ox, oy = self.display_offset
        scale = self.display_scale
        img_x = int((cx - ox) / scale)
        img_y = int((cy - oy) / scale)
        # clamp
        if self.current is None:
            return (img_x, img_y)
        h, w = self.current.shape[:2]
        img_x = max(0, min(w - 1, img_x))
        img_y = max(0, min(h - 1, img_y))
        return (img_x, img_y)

    def canvas_coords_from_image(self, ix, iy):
        ox, oy = self.display_offset
        scale = self.display_scale
        cx = int(ix * scale + ox)
        cy = int(iy * scale + oy)
        return (cx, cy)

    # --------- Início do movimento / clique no canvas ---------
    def on_canvas_click(self, event):
        # apenas modo foto permite stickers
        if self.mode != 'photo':
            return

        if self.current is None:
            return

        # se sticker selecionado for "Nenhum", não inicia nada (usado para evitar aplicar sem querer)
        sticker_key = self.selected_sticker_name.get()
        if not sticker_key or sticker_key == 'Nenhum':
            # Mas se há um sticker em preview (por segurança), cancela
            return

        # mapeia clique para coordenadas da imagem (não do canvas)
        ix, iy = self.image_coords_from_canvas(event.x, event.y)

        # prepara movimento/resize
        sticker_info = self.stickers.get(sticker_key)
        if sticker_info is None:
            return

        # estabelece imagem do sticker (BGRA)
        fg = sticker_info['img']
        fh, fw = fg.shape[:2]

        # posição inicial: colocamos o sticker com o canto superior esquerdo onde clicou
        x0, y0 = ix, iy
        w0, h0 = fw, fh

        # se já existe um sticker sendo mostrado (preview), detecta se o clique foi no canto para redimensionar
        # se usuário clicar próximo ao canto inferior-direito do sticker, inicia resize.
        # iniciamos move por padrão (coloca sticker com canto superior onde clicou).
        self.moving = True
        self.moving_mode = 'move'  # por padrão
        self.moving_sticker_key = sticker_key
        self.moving_sticker_img = fg.copy()
        self.moving_sticker_pos = (x0, y0)
        self.moving_sticker_size = (w0, h0)
        self.temp_image = self.current.copy()

        # mostra preview inicial
        self.show_sticker_preview_image(self.moving_sticker_img, self.moving_sticker_pos, self.moving_sticker_size)

    def on_canvas_drag(self, event):
        if not self.moving:
            return
        if self.current is None:
            return

        # mapeia para coordenadas da imagem
        ix, iy = self.image_coords_from_canvas(event.x, event.y)

        # se modo move: atualiza posição do canto superior esquerdo proporcionalmente
        if self.moving_mode == 'move':
            x, y = ix, iy
            self.moving_sticker_pos = (x, y)
            self.show_sticker_preview_image(self.moving_sticker_img, self.moving_sticker_pos, self.moving_sticker_size)
        elif self.moving_mode == 'resize':
            # resize baseado em canto superior esquerdo fixo
            x0, y0 = self.moving_sticker_pos
            new_w = max(1, ix - x0)
            new_h = max(1, iy - y0)
            self.moving_sticker_size = (new_w, new_h)
            self.show_sticker_preview_image(self.moving_sticker_img, self.moving_sticker_pos, self.moving_sticker_size)

    def on_canvas_release(self, event):
        if not self.moving:
            return
        if self.current is None:
            self.moving = False
            return
        # ao soltar, aplica sticker permanente sobre current
        x_img, y_img = self.moving_sticker_pos
        w, h = self.moving_sticker_size
        fg = self.moving_sticker_img

        # redimensiona fg para w,h mantendo canais
        if (fg.shape[1], fg.shape[0]) != (w, h):
            if w <= 0 or h <= 0:
                self.moving = False
                return
            resized = cv2.resize(fg, (w, h), interpolation=cv2.INTER_AREA)
        else:
            resized = fg

        self.current = overlay_alpha(self.current, resized, x_img, y_img)
        self.moving = False
        self.moving_mode = None
        self.moving_sticker_key = None
        self.moving_sticker_img = None
        self.temp_image = None
        self._refresh_canvas()

    def show_sticker_preview_image(self, fg_rgba, pos, size):
        """
        Exibe preview do sticker durante arraste.
        pos = (x_img, y_img) coordenadas na imagem (não canvas).
        size = (w,h)
        """
        if self.current is None:
            return
        temp = self.temp_image.copy()
        w, h = size
        if w <= 0 or h <= 0:
            self.display_image(temp)
            return
        # redimensiona mantendo canais (BGRA)
        if (fg_rgba.shape[1], fg_rgba.shape[0]) != (w, h):
            fg_resized = cv2.resize(fg_rgba, (w, h), interpolation=cv2.INTER_AREA)
        else:
            fg_resized = fg_rgba
        preview = overlay_alpha(temp, fg_resized, pos[0], pos[1])
        self.display_image(preview)

    # --------- Câmera ---------
    def start_camera(self):
        if self.cam_running:
            return
        self.cap = cv2.VideoCapture(0)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAM_WIDTH)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT)
        if not self.cap.isOpened():
            messagebox.showerror('Webcam', 'Não foi possível abrir a webcam')
            return
        self.cam_running = True
        self.mode = 'video'
        self.cam_thread = threading.Thread(target=self._camera_loop, daemon=True)
        self.cam_thread.start()

    def stop_camera(self):
        self.cam_running = False
        if self.cap:
            try:
                self.cap.release()
            except Exception:
                pass
            self.cap = None

    def _camera_loop(self):
        while self.cam_running and self.cap and self.cap.isOpened():
            ret, frame = self.cap.read()
            if not ret:
                break
            fname = self.selected_filter.get()
            out = apply_filter_cv(frame, fname)
            # só dá update na current, nao muda a original
            self.current = out
            self._refresh_canvas_from_cv(out)
        self.cam_running = False

    def display_image(self, img_bgr):
        self._refresh_canvas_from_cv(img_bgr)

    # --------- Update da tela ---------
    def _refresh_canvas(self):
        if self.current is None:
            self.canvas.delete('all')
            return
        self._refresh_canvas_from_cv(self.current)

    def _refresh_canvas_from_cv(self, img_bgr):
        """
        Exibe img_bgr no canvas dimensionando para CAM_WIDTH x CAM_HEIGHT mantendo aspect ratio.
        Armazena os valores de scale e offset para mapear cliques.
        """
        h, w = img_bgr.shape[:2]
        scale = min(CAM_WIDTH / w, CAM_HEIGHT / h)
        nw = int(w * scale)
        nh = int(h * scale)
        img_resized = cv2.resize(img_bgr, (nw, nh))
        pil = cv2_to_pil(img_resized)
        photo = ImageTk.PhotoImage(pil)
        self._photoimage = photo
        self.canvas.delete('all')
        x = (CAM_WIDTH - nw) // 2
        y = (CAM_HEIGHT - nh) // 2
        self.canvas.create_image(x, y, anchor=tk.NW, image=photo)

        # guarda dados para mapeamento
        self.display_scale = scale
        self.display_offset = (x, y)
        self.display_size = (nw, nh)

    # --------- Ajuda e sair ---------
    def show_help(self):
        txt = (
            'Controles:\n'
            '- Carregar imagem: abre uma imagem (Modo foto)\n'
            '- Modo vídeo: abre a webcam e aplica o filtro selecionado ao vivo\n'
            '- Aplicar filtro: aplica o filtro selecionado à imagem atual (filtros com valores fixos)\n'
            '- Checkbox: escolher se o filtro aplica sempre sobre o ORIGINAL (não empilha)\n'
            '- Stickers: selecione um sticker e clique na imagem para posicioná-lo (apenas Modo foto)\n'
            '   - Arraste para mover o sticker\n'
            '   - Arraste no canto inferior-direito do sticker para redimensionar (diagonal)\n'
            '- Operações: carregue uma 2ª imagem e use Add/Sub/Blend\n'
            '- Salvar: salva a imagem atual em disco'
        )
        messagebox.showinfo('Ajuda', txt)

    def on_close(self):
        if self.cam_running:
            self.stop_camera()
        self.root.quit()
        self.root.destroy()

# ------------------- Main -------------------

def main():
    root = tk.Tk()
    app = EditorApp(root)
    root.protocol('WM_DELETE_WINDOW', app.on_close)
    root.mainloop()

if __name__ == '__main__':
    main()
