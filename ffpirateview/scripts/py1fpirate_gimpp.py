#!/usr/bin/env python
from gimpfu import *
import struct

MAGIC = b'1bfarbfe'

def save_1fpirate_file(image, drawable, filename, raw_filename):
    width = image.width
    height = image.height
    row_bytes = (width + 7) // 8
    bitmap = bytearray(row_bytes * height)

    # Get grayscale values
    region = drawable.get_pixel_rgn(0, 0, width, height, False)
    pixels = region[0:width, 0:height]

    for y in range(height):
        for x in range(width):
            pixel = pixels[y * width + x]
            gray = pixel if isinstance(pixel, int) else pixel[0]  # handles byte or bytearray
            bit = 0 if gray > 127 else 1
            byte_index = y * row_bytes + (x // 8)
            bit_index = 7 - (x % 8)
            if bit:
                bitmap[byte_index] |= (1 << bit_index)

    with open(filename, 'wb') as f:
        f.write(b'1bfarbfe')
        f.write(struct.pack('>I', width))
        f.write(struct.pack('>I', height))
        f.write(bitmap)

    return

register(
    "python_fu_save_1fpirate",
    "Save 1fpirate image",
    "Saves image in 1-bit 1fpirate (.1f) format",
    "tcooper", "tcooper", "2025",
    "1fpirate image",
    "*",  # Works for any image type
    [
        (PF_IMAGE, "image", "Input image", None),
        (PF_DRAWABLE, "drawable", "Input drawable", None),
        (PF_STRING, "filename", "Output filename", None),
        (PF_STRING, "raw_filename", "Raw filename", None)
    ],
    [],
    save_1fpirate_file,
    menu="<Save>"
    gimp.register_save_handler("python_fu_save_1fpirate", "1f,1ff", "")
)



def read_1f_header(fp):
    magic = fp.read(8)
    if magic != MAGIC:
        raise ValueError("Invalid 1fpirate magic")
    width = struct.unpack(">I", fp.read(4))[0]
    height = struct.unpack(">I", fp.read(4))[0]
    return width, height

def load_1fpirate_file(filename, raw_filename):
    with open(filename, 'rb') as f:
        width, height = read_1f_header(f)
        row_bytes = (width + 7) // 8
        bitmap = f.read(row_bytes * height)

    # Create GIMP image and layer
    img = gimp.Image(width, height, GRAY)
    layer = gimp.Layer(img, "1fpirate layer", width, height, GRAY_IMAGE, 100, NORMAL_MODE)
    img.add_layer(layer, 0)
    region = layer.get_pixel_rgn(0, 0, width, height, True)

    # Convert 1-bit image to grayscale: white = 255, black = 0
    pixels = bytearray(width * height)
    for y in range(height):
        for x in range(width):
            byte_index = y * row_bytes + (x // 8)
            bit_index = 7 - (x % 8)
            bit = (bitmap[byte_index] >> bit_index) & 1
            pixels[y * width + x] = 0 if bit else 255

    region[0:width, 0:height] = bytes(pixels)
    layer.flush()
    layer.merge_shadow(True)
    layer.update(0, 0, width, height)
    return img

register(
    "python_fu_load_1fpirate",
    "Load 1fpirate image",
    "Loads .1f or .1ff 1-bit farbfeld-style images",
    "tcooper", "tcooper", "2025",
    "<Image>/File/Open 1fpirate...",
    "",  # No image required to run
    [
        (PF_STRING, "filename", "The name of the file", None),
        (PF_STRING, "raw_filename", "The raw filename", None)
    ],
    [],
    load_1fpirate_file,
    on_query=None
)

main()
