"""
Editor de imagens e vídeo - Trabalho de GB
Aluna: Gabriela Bley Rodrigues
Prof: Rossana Baptista
"""

import os
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from PIL import Image, ImageTk
import cv2
import numpy as np

# ------------------- Config -------------------
STICKERS_DIR = 'stickers'
WINDOW_TITLE = 'Editor de Imagens e Vídeo - Grau A'
CAM_WIDTH = 640
CAM_HEIGHT = 480

# ------------------- Filtros -------------------
FILTERS = [
    'none', 'gaussian', 'box', 'median', 'bilateral',
    'sharpen', 'unsharp', 'laplacian', 'emboss', 'canny', 'hist_eq'
]

FILTER_INFO = {
    'none': 'Sem filtro.',
    'gaussian': 'Suavização Gaussiana (5x5). Remove ruído suave.',
    'box': 'Blur Box (média 5x5). Suavização simples.',
    'median': 'MedianBlur (k=5). Remove ruído "sal-e-pimenta".',
    'bilateral': 'Filtro bilateral: suaviza mantendo as bordas.',
    'sharpen': 'Realça bordas (matriz de nitidez).',
    'unsharp': 'Unsharp mask (realce subtraindo a versão borrada).',
    'laplacian': 'Laplaciano: detecta bordas (segunda derivada).',
    'emboss': 'Emboss: cria efeito de relevo/sombra.',
    'canny': 'Canny: Detector de bordas otimizado.',
    'hist_eq': 'Equalização de Histograma (melhora contraste).'
}

# ------------------- Utils: processamento -------------------

def apply_filter_cv(img_bgr, name):
    """Aplica o filtro e retorna imagem BGR."""
    if img_bgr is None: return None
    img = img_bgr.copy()
    
    if name == 'none': return img
    
    # conversões necessárias para filtros que exigem Gray na entrada
    if name in ['canny', 'hist_eq', 'laplacian']:
        # se já for colorido, converte para processar, depois volta para BGR para manter padrão
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        if name == 'canny':
            edges = cv2.Canny(gray, 50, 150)
            return cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        if name == 'hist_eq':
            eq = cv2.equalizeHist(gray)
            return cv2.cvtColor(eq, cv2.COLOR_GRAY2BGR)
        if name == 'laplacian':
            lap = cv2.Laplacian(gray, cv2.CV_64F)
            lap = cv2.convertScaleAbs(lap)
            return cv2.cvtColor(lap, cv2.COLOR_GRAY2BGR)

    # filtros que aceitam BGR direto
    if name == 'gaussian': return cv2.GaussianBlur(img, (5,5), 1.0)
    if name == 'box': return cv2.blur(img, (5,5))
    if name == 'median': return cv2.medianBlur(img, 5)
    if name == 'bilateral': return cv2.bilateralFilter(img, 9, 75, 75)
    
    if name == 'sharpen':
        kernel = np.array([[0,-1,0], [-1,5,-1], [0,-1,0]], dtype=np.float32)
        return cv2.filter2D(img, -1, kernel)
    
    if name == 'unsharp':
        blurred = cv2.GaussianBlur(img, (9,9), 10.0)
        return cv2.addWeighted(img, 1.5, blurred, -0.5, 0)
    
    if name == 'emboss':
        kernel = np.array([[-2,-1,0], [-1,1,1], [0,1,2]], dtype=np.float32)
        emb = cv2.filter2D(img, -1, kernel) + 128
        return cv2.convertScaleAbs(emb)

    return img

def get_channel_view(img_bgr, channel_mode):
    """
    Retorna a visualização baseada no canal selecionado.
    channel_mode: 'RGB', 'Gray', 'Red', 'Green', 'Blue'
    """
    if img_bgr is None: return None
    if channel_mode == 'RGB':
        return img_bgr
    elif channel_mode == 'Gray':
        gray = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2GRAY)
        return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)
    
    # separa canais (openCV usa BGR)
    b, g, r = cv2.split(img_bgr)
    zeros = np.zeros_like(b)
    
    if channel_mode == 'Red':
        return cv2.merge([zeros, zeros, r])
    elif channel_mode == 'Green':
        return cv2.merge([zeros, g, zeros])
    elif channel_mode == 'Blue':
        return cv2.merge([b, zeros, zeros])
    
    return img_bgr

# ------------------- Utils: stickers -------------------
def load_stickers(dirpath, size=(300, 300)):
    stickers = {}
    if not os.path.exists(dirpath):
        try:
            os.makedirs(dirpath)
            print(f"Pasta '{dirpath}' criada. Coloque imagens PNG lá.")
        except:
            pass
        return stickers

    for fname in sorted(os.listdir(dirpath)):
        if fname.lower().endswith('.png'):
            path = os.path.join(dirpath, fname)
            im = cv2.imread(path, cv2.IMREAD_UNCHANGED)
            if im is None: continue
            
            # resize inicial se for muito grande
            h, w = im.shape[:2]
            max_side = max(w, h)
            if max_side > size[0]:
                scale = size[0] / max_side
                im = cv2.resize(im, (int(w*scale), int(h*scale)), interpolation=cv2.INTER_AREA)
            
            clean_name = os.path.splitext(fname)[0]
            stickers[clean_name] = im
    return stickers

def overlay_alpha(bg, fg_rgba, x, y):
    """Sobrepõe sticker RGBA no BG (BGR) na posição x,y."""
    out = bg.copy()
    bh, bw = out.shape[:2]
    fh, fw = fg_rgba.shape[:2]
    
    if x >= bw or y >= bh: return out
    
    # clip coordinates
    x1, y1 = max(x, 0), max(y, 0)
    x2, y2 = min(x + fw, bw), min(y + fh, bh)
    
    # offsets no sticker (caso x ou y sejam negativos)
    fx1 = max(0, -x)
    fy1 = max(0, -y)
    fx2 = fx1 + (x2 - x1)
    fy2 = fy1 + (y2 - y1)
    
    if (x2 <= x1) or (y2 <= y1): return out

    fg_crop = fg_rgba[fy1:fy2, fx1:fx2]
    bg_crop = out[y1:y2, x1:x2]

    if fg_crop.shape[2] == 4:
        alpha = fg_crop[:, :, 3].astype(np.float32) / 255.0
        for c in range(3):
            bg_crop[:, :, c] = (alpha * fg_crop[:, :, c] + (1 - alpha) * bg_crop[:, :, c])
    else:
        bg_crop[:] = fg_crop

    out[y1:y2, x1:x2] = bg_crop
    return out

# ------------------- Interface -------------------

class EditorApp:
    def __init__(self, root):
        self.root = root
        self.root.title(WINDOW_TITLE)

        # estado
        self.mode = 'photo' 
        self.cap = None
        self.cam_running = False

        self.original = None
        self.current = None
        self.second_image = None 

        # variáveis UI
        self.selected_filter = tk.StringVar(value='none')
        self.selected_channel = tk.StringVar(value='RGB') # Novo requisito
        self.filter_overwrite = tk.BooleanVar(value=True)
        
        # stickers
        self.stickers = load_stickers(STICKERS_DIR)
        self.sticker_labels = ['Nenhum'] + list(self.stickers.keys())
        self.selected_sticker = tk.StringVar(value='Nenhum')

        # interação canvas
        self.display_params = {'scale': 1.0, 'offset': (0,0)}
        self.moving = False
        self.move_data = {} # armazena estado do arraste

        self._build_ui()
        
        # cria imagem preta padrão para iniciar bonito
        blank = np.zeros((CAM_HEIGHT, CAM_WIDTH, 3), dtype=np.uint8)
        self.original = blank
        self.current = blank.copy()
        self._refresh_canvas()

    def _build_ui(self):
        # layout: esquerda fica o canvas, direita ficam os controles
        frame_left = tk.Frame(self.root)
        frame_left.pack(side=tk.LEFT, padx=10, pady=10)
        
        self.canvas = tk.Canvas(frame_left, width=CAM_WIDTH, height=CAM_HEIGHT, bg='#222')
        self.canvas.pack()
        # bindings do mouse
        self.canvas.bind('<Button-1>', self.on_canvas_click)
        self.canvas.bind('<B1-Motion>', self.on_canvas_drag)
        self.canvas.bind('<ButtonRelease-1>', self.on_canvas_release)

        frame_right = tk.Frame(self.root)
        frame_right.pack(side=tk.RIGHT, fill=tk.Y, padx=10, pady=10)

        # modos
        lbl_mode = tk.LabelFrame(frame_right, text="Modo de Operação")
        lbl_mode.pack(fill=tk.X, pady=5)
        tk.Button(lbl_mode, text="Modo Foto (Carregar)", command=self.btn_load_img).pack(side=tk.LEFT, padx=5, pady=5)
        tk.Button(lbl_mode, text="Modo Vídeo (Webcam)", command=self.btn_toggle_cam).pack(side=tk.LEFT, padx=5, pady=5)

        # canais
        lbl_chan = tk.LabelFrame(frame_right, text="Visualização de Canais")
        lbl_chan.pack(fill=tk.X, pady=5)
        cbox_chan = ttk.Combobox(lbl_chan, values=['RGB', 'Red', 'Green', 'Blue', 'Gray'], 
                                 textvariable=self.selected_channel, state='readonly')
        cbox_chan.pack(fill=tk.X, padx=5, pady=5)
        cbox_chan.bind('<<ComboboxSelected>>', lambda e: self._refresh_canvas())

        # filtros
        lbl_filt = tk.LabelFrame(frame_right, text="Filtros")
        lbl_filt.pack(fill=tk.X, pady=5)
        
        cbox_filt = ttk.Combobox(lbl_filt, values=FILTERS, textvariable=self.selected_filter, state='readonly')
        cbox_filt.pack(fill=tk.X, padx=5, pady=5)
        cbox_filt.bind('<<ComboboxSelected>>', self.on_filter_info)
        
        self.lbl_desc = tk.Label(lbl_filt, text="Descrição...", wraplength=250, justify=tk.LEFT, fg="gray")
        self.lbl_desc.pack(fill=tk.X, padx=5)

        tk.Button(lbl_filt, text="Aplicar Filtro", command=self.btn_apply_filter).pack(fill=tk.X, padx=5, pady=5)
        tk.Checkbutton(lbl_filt, text="Resetar ao aplicar (não empilhar)", variable=self.filter_overwrite).pack(anchor='w')

        # aritmética
        lbl_ari = tk.LabelFrame(frame_right, text="Operações (2 Imagens)")
        lbl_ari.pack(fill=tk.X, pady=5)
        tk.Button(lbl_ari, text="Carregar 2ª Imagem", command=self.btn_load_second).pack(fill=tk.X, padx=5)
        f_ari_btns = tk.Frame(lbl_ari)
        f_ari_btns.pack(fill=tk.X)
        tk.Button(f_ari_btns, text="Soma", command=lambda: self.do_arith('add')).pack(side=tk.LEFT, expand=True)
        tk.Button(f_ari_btns, text="Sub", command=lambda: self.do_arith('sub')).pack(side=tk.LEFT, expand=True)
        tk.Button(f_ari_btns, text="Blend", command=lambda: self.do_arith('blend')).pack(side=tk.LEFT, expand=True)

        # stickers
        lbl_stk = tk.LabelFrame(frame_right, text="Stickers (Modo Foto)")
        lbl_stk.pack(fill=tk.X, pady=5)
        self.combo_stickers = ttk.Combobox(lbl_stk, values=self.sticker_labels, textvariable=self.selected_sticker, state='readonly')
        self.combo_stickers.pack(fill=tk.X, padx=5, pady=5)
        tk.Label(lbl_stk, text="Selecione e clique na imagem.\nArraste p/ mover, Canto Inf-Dir p/ redim.", font=("Arial", 8)).pack()

        # controle geral
        tk.Button(frame_right, text="Resetar Tudo", command=self.btn_reset).pack(fill=tk.X, pady=5)
        tk.Button(frame_right, text="Salvar Imagem", command=self.btn_save).pack(fill=tk.X, pady=5)

    # ------------------- Lógica de câmera -------------------
    def btn_toggle_cam(self):
        if self.cam_running:
            self.stop_cam()
        else:
            self.start_cam()
    
    def start_cam(self):
        self.cap = cv2.VideoCapture(0)
        if not self.cap.isOpened():
            messagebox.showerror("Erro", "Não foi possível abrir a webcam.")
            return
        self.cap.set(3, CAM_WIDTH)
        self.cap.set(4, CAM_HEIGHT)
        self.cam_running = True
        self.mode = 'video'
        self.update_cam_frame() # inicia loop via root.after
    
    def stop_cam(self):
        self.cam_running = False
        if self.cap:
            self.cap.release()
        self.cap = None
        self.mode = 'photo'

    def update_cam_frame(self):
        if not self.cam_running: return
        
        ret, frame = self.cap.read()
        if ret:
            # espelhar webcam
            frame = cv2.flip(frame, 1)
            
            # se tiver filtro selecionado no modo vídeo, aplica em tempo real
            filt = self.selected_filter.get()
            processed = apply_filter_cv(frame, filt)
            
            self.current = processed
            self._refresh_canvas()
        
        # agenda a próxima execução em 10ms (aprox 30-60fps)
        self.root.after(15, self.update_cam_frame)

    # ------------------- Ações de botão -------------------
    def btn_load_img(self):
        self.stop_cam()
        path = filedialog.askopenfilename(filetypes=[("Imagens", "*.jpg *.png *.jpeg *.bmp")])
        if path:
            img = cv2.imread(path)
            if img is not None:
                self.original = img
                self.current = img.copy()
                self.mode = 'photo'
                self._refresh_canvas()

    def btn_save(self):
        if self.current is None: return
        # ao salvar, salvar o que está sendo visto 
        to_save = get_channel_view(self.current, self.selected_channel.get())
        path = filedialog.asksaveasfilename(defaultextension=".jpg", filetypes=[("JPG", "*.jpg"), ("PNG", "*.png")])
        if path:
            cv2.imwrite(path, to_save)
            messagebox.showinfo("Salvo", f"Imagem salva em {path}")

    def btn_reset(self):
        if self.original is not None:
            self.current = self.original.copy()
            self._refresh_canvas()

    def on_filter_info(self, event):
        d = FILTER_INFO.get(self.selected_filter.get(), "")
        self.lbl_desc.config(text=d)

    def btn_apply_filter(self):
        if self.mode == 'video':
            messagebox.showinfo("Info", "No modo vídeo o filtro é aplicado em tempo real.")
            return
        
        if self.filter_overwrite.get():
            base = self.original.copy()
        else:
            base = self.current.copy()
            
        res = apply_filter_cv(base, self.selected_filter.get())
        self.current = res
        self._refresh_canvas()

    # ------------------- Aritmética -------------------
    def btn_load_second(self):
        path = filedialog.askopenfilename(title="Segunda Imagem")
        if path:
            self.second_image = cv2.imread(path)
            messagebox.showinfo("Ok", "Segunda imagem carregada!")

    def do_arith(self, op):
        if self.current is None or self.second_image is None:
            messagebox.showwarning("Aviso", "Precisa da imagem principal e da 2ª imagem.")
            return
        
        # redimensiona a segunda para caber na primeira
        h, w = self.current.shape[:2]
        img2 = cv2.resize(self.second_image, (w, h))
        
        if op == 'add':
            self.current = cv2.add(self.current, img2)
        elif op == 'sub':
            self.current = cv2.subtract(self.current, img2)
        elif op == 'blend':
            self.current = cv2.addWeighted(self.current, 0.5, img2, 0.5, 0)
        
        self._refresh_canvas()

    # ------------------- Canvas e stickers -------------------
    def _refresh_canvas(self):
        if self.current is None: return
        
        # aplica a visualização de canal (RGB, R, G, B ou Gray)
        view_img = get_channel_view(self.current, self.selected_channel.get())
        
        # converte para exibir no Tkinter
        h, w = view_img.shape[:2]
        
        # lógica de redimensionar para caber na tela mantendo proporção
        scale = min(CAM_WIDTH/w, CAM_HEIGHT/h)
        nw, nh = int(w*scale), int(h*scale)
        
        resized = cv2.resize(view_img, (nw, nh))
        
        rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
        pil_img = Image.fromarray(rgb)
        self.tk_img = ImageTk.PhotoImage(pil_img) # guardar ref para não perder
        
        self.canvas.delete("all")
        # centralizar
        off_x = (CAM_WIDTH - nw) // 2
        off_y = (CAM_HEIGHT - nh) // 2
        
        self.canvas.create_image(off_x, off_y, anchor=tk.NW, image=self.tk_img)
        
        # salva dados pra converter coordenadas do mouse depois
        self.display_params = {'scale': scale, 'offset': (off_x, off_y), 'size': (nw, nh)}

    # --- Mouse events pros stickers ---
    def get_img_coords(self, cx, cy):
        """Converte X,Y do Canvas para X,Y da imagem real"""
        scale = self.display_params['scale']
        ox, oy = self.display_params['offset']
        ix = int((cx - ox) / scale)
        iy = int((cy - oy) / scale)
        return ix, iy

    def on_canvas_click(self, event):
        if self.mode != 'photo': return
        stk_name = self.selected_sticker.get()
        if stk_name == 'Nenhum': return

        ix, iy = self.get_img_coords(event.x, event.y)
        
        # começa movimentação
        stk_img = self.stickers[stk_name]
        self.moving = True
        self.move_data = {
            'img': stk_img,
            'pos': (ix, iy),
            'base_img': self.current.copy(), # backup para o preview não estragar a imagem
            'size': (stk_img.shape[1], stk_img.shape[0]),
            'mode': 'move' # ou resize
        }
        self.draw_sticker_preview()

    def on_canvas_drag(self, event):
        if not self.moving: return
        ix, iy = self.get_img_coords(event.x, event.y)
        
        # Lógica simples: se clicar e arrastar, move o topo-esquerdo
        # Se quisesse redimensionar, checaria se o clique foi perto da borda inferior direita
        
        # Detectar modo resize (se estiver perto do canto inferior direito do sticker atual)
        mx, my = self.move_data['pos']
        mw, mh = self.move_data['size']
        
        dist_corner = ((ix - (mx+mw))**2 + (iy - (my+mh))**2)**0.5
        if dist_corner < 50 or self.move_data['mode'] == 'resize':
             self.move_data['mode'] = 'resize'
             nw = max(10, ix - mx)
             nh = max(10, iy - my)
             self.move_data['size'] = (nw, nh)
        else:
            self.move_data['mode'] = 'move'
            self.move_data['pos'] = (ix, iy)
            
        self.draw_sticker_preview()

    def on_canvas_release(self, event):
        if not self.moving: return
        self.moving = False
        
        # aplica definitivamente
        stk = self.move_data['img']
        w, h = self.move_data['size']
        stk_resized = cv2.resize(stk, (w, h))
        
        x, y = self.move_data['pos']
        
        # aplica na imagem basee que tava salva
        final_img = overlay_alpha(self.move_data['base_img'], stk_resized, x, y)
        self.current = final_img
        self._refresh_canvas()

    def draw_sticker_preview(self):
        # pega a imagem limpa
        temp = self.move_data['base_img'].copy()
        stk = self.move_data['img']
        w, h = self.move_data['size']
        x, y = self.move_data['pos']
        
        stk_resized = cv2.resize(stk, (w, h))
        preview = overlay_alpha(temp, stk_resized, x, y)
        
        self.current = preview 
        self._refresh_canvas()

# ------------------- main -------------------
if __name__ == '__main__':
    root = tk.Tk()
    app = EditorApp(root)
    root.mainloop()