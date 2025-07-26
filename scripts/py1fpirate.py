import sys
from tkinter import Tk, Label
from PIL import Image, ImageTk

MAGIC = b'1fpirate'

def load_1ff(filename_or_fp):
    if hasattr(filename_or_fp, 'read'):
        fp = filename_or_fp
    else:
        fp = open(filename_or_fp, 'rb')

    magic = fp.read(8)
    if magic != MAGIC:
        raise ValueError("Not a valid 1ff file")

    width = int.from_bytes(fp.read(4), 'big')
    height = int.from_bytes(fp.read(4), 'big')
    row_bytes = (width + 7) // 8
    bitmap = fp.read(row_bytes * height)

    img = Image.new('1', (width, height))
    pixels = img.load()
    for y in range(height):
        for x in range(width):
            byte = bitmap[y * row_bytes + (x // 8)]
            bit = (byte >> (7 - (x % 8))) & 1
            pixels[x, y] = 255 if bit == 1 else 0

    return img.convert("L")  # convert to grayscale to allow resizing

class Viewer:
    def __init__(self, img):
        self.root = Tk()
        self.root.title("1ff Viewer")
        self.img = img
        self.zoom = 1.0
        self.tk_img = None
        self.label = Label(self.root)
        self.label.pack()
        self.update_image()

        #self.root.bind("<Escape>", lambda e: self.root.destroy())
        self.root.bind("q", lambda e: self.root.destroy())  # Quit on 'q'
        self.root.bind("+", self.zoom_in)                   # Zoom in
        self.root.bind("=", self.zoom_in)    #dont have to press shift
        self.root.bind("-", self.zoom_out)                  # Zoom out


        #self.root.bind("<KeyPress-minus>", self.zoom_out)
        #self.root.bind("<KeyPress-=>", self.zoom_in)
        #self.root.bind("<KeyPress-plus>", self.zoom_in)  # For some keyboards

    def update_image(self):
        w, h = self.img.size
        size = (int(w * self.zoom), int(h * self.zoom))
        resized = self.img.resize(size)
        self.tk_img = ImageTk.PhotoImage(resized)
        self.label.config(image=self.tk_img)

    def zoom_in(self, event=None):
        self.zoom *= 1.25
        self.update_image()

    def zoom_out(self, event=None):
        self.zoom /= 1.25
        self.update_image()

    def run(self):
        self.root.mainloop()

if __name__ == '__main__':
    if len(sys.argv) == 1:
        img = load_1ff(sys.stdin.buffer)
    else:
        img = load_1ff(sys.argv[1])
    Viewer(img).run()
