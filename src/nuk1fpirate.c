/*
 * Nuklear-based version of ffpirate.c - farbfeld/1f image viewer
 * This version replaces SDL2 with Nuklear+X11
 */

#define NK_IMPLEMENTATION
#define NK_XLIB_IMPLEMENTATION

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "nuklear.h"
#include "nuklear_xlib.h"

#define MAX_IMAGE_WIDTH 4096
#define MAX_IMAGE_HEIGHT 4096
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct nk_context *ctx;
Display *dpy;
Window win;
Atom wm_delete_window;
Pixmap framebuffer;
GC gc;
int screen;

uint32_t width, height;
uint8_t *bitmap = NULL;
uint32_t *pixels = NULL;
int inverted_colors = 0;

void draw_image(struct nk_context *ctx, GC gc, Pixmap framebuffer, int x, int y) {
    XImage *img;
    img = XCreateImage(dpy, DefaultVisual(dpy, screen), 24, ZPixmap, 0,
                       (char *)pixels, width, height, 32, 0);
    if (img) {
        XPutImage(dpy, framebuffer, gc, img, 0, 0, x, y, width, height);
        img->data = NULL; // Prevent free of pixels
        XDestroyImage(img);
    }
}

void update_pixels() {
    int row_bytes = (width + 7) / 8;
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            int byte_index = y * row_bytes + (x / 8);
            int bit_index = 7 - (x % 8);
            int bit = (bitmap[byte_index] >> bit_index) & 1;
            pixels[y * width + x] = bit ? 0x000000 : 0xFFFFFF;
        }
    }
}

int load_1f(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return -1;

    char magic[8];
    fread(magic, 1, 8, fp);
    if (memcmp(magic, "1bfarbfe", 8) != 0) {
        fclose(fp);
        return -1;
    }

    fread(&width, sizeof(uint32_t), 1, fp);
    fread(&height, sizeof(uint32_t), 1, fp);
    width = ntohl(width);
    height = ntohl(height);

    int row_bytes = (width + 7) / 8;
    int size = row_bytes * height;

    bitmap = malloc(size);
    fread(bitmap, 1, size, fp);
    fclose(fp);

    pixels = malloc(width * height * sizeof(uint32_t));
    update_pixels();

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.1f>\n", argv[0]);
        return 1;
    }
    if (load_1f(argv[1]) != 0) {
        fprintf(stderr, "Failed to load %s\n", argv[1]);
        return 1;
    }

    dpy = XOpenDisplay(NULL);
    screen = DefaultScreen(dpy);
    win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen), 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 1,
                              BlackPixel(dpy, screen), WhitePixel(dpy, screen));

    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wm_delete_window, 1);

    XStoreName(dpy, win, "1fpirate-nuklear");
    XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
    XMapWindow(dpy, win);

    framebuffer = XCreatePixmap(dpy, win, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, DefaultDepth(dpy, screen));
    gc = XCreateGC(dpy, framebuffer, 0, NULL);

    ctx = nk_xlib_init(dpy, screen, win, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));

    while (1) {
        XEvent evt;
        while (XPending(dpy)) {
            XNextEvent(dpy, &evt);
            if (evt.type == ClientMessage && evt.xclient.data.l[0] == (long)wm_delete_window) {
                goto cleanup;
            }
            nk_xlib_handle_event(&evt);
        }

        nk_xlib_new_frame();

        if (nk_begin(ctx, "Image", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_dynamic(ctx, height, 1);
            draw_image(ctx, gc, framebuffer, 0, 0);
        }
        nk_end(ctx);

        XCopyArea(dpy, framebuffer, win, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
        nk_xlib_render(NK_ANTI_ALIASING_ON);
    }

cleanup:
    nk_xlib_shutdown();
    XFreeGC(dpy, gc);
    XFreePixmap(dpy, framebuffer);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    free(pixels);
    free(bitmap);
    return 0;
}
