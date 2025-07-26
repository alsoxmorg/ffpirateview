/*xtpirate is a completly AI coded motif toolkit farbfeld viewer
 it is very buggy (like crashes on pan) but its proof of concept.
 you can build with cc -o ffview xtpirate2.c -lXm -lXt -lX11

*/

#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <X11/keysym.h>
#include <Xm/Form.h>
#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t width, height;
    uint16_t *pixels; // RGBA 16-bit
} FarbfeldImage;

Display *dpy;
Window win;
GC gc;
Widget drawing_area;
FarbfeldImage *img = NULL;
float zoom = 1.0;
int locx = 0;
int locy = 0;

FarbfeldImage *load_farbfeld(const char *filename);
void free_farbfeld(FarbfeldImage *img);
void draw_image(Widget w, XtPointer client_data, XtPointer call_data);
void key_handler(Widget w, XtPointer client_data, XEvent *event, Boolean *ctd);

int main(int argc, char **argv) {
    XtAppContext app;
    Widget top, shell;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s image.ff\n", argv[0]);
        return 1;
    }

    img = load_farbfeld(argv[1]);
    if (!img) {
        fprintf(stderr, "Failed to load Farbfeld image\n");
        return 1;
    }

    top = XtVaAppInitialize(&app, "FFView", NULL, 0, &argc, argv, NULL, NULL);

    shell = XtVaCreateManagedWidget("shell", xmFormWidgetClass, top, NULL);

    drawing_area = XtVaCreateManagedWidget("drawing", xmDrawingAreaWidgetClass, shell,
                                           XmNwidth, img->width,
                                           XmNheight, img->height,
                                           NULL);

    XtAddCallback(drawing_area, XmNexposeCallback, draw_image, NULL);
    XtAddEventHandler(drawing_area, KeyPressMask, False, key_handler, NULL);

    XtRealizeWidget(top);

    dpy = XtDisplay(drawing_area);
    win = XtWindow(drawing_area);
    gc = DefaultGC(dpy, DefaultScreen(dpy));

    XtAppMainLoop(app);
    return 0;
}

void key_handler(Widget w, XtPointer client_data, XEvent *event, Boolean *ctd) {
    KeySym key = XLookupKeysym(&event->xkey, 0);
    if (key == XK_v || key == XK_KP_Add) {
        zoom *= 1.25;
    } else if (key == XK_c || key == XK_KP_Subtract) {
        zoom /= 1.25;
        if (zoom < 0.1) zoom = 0.1;
    } else if (key == XK_q ) {
        //exit
    } else if (key == XK_Left) {
        locx-= 1;
    } else if (key == XK_Right) {
        locx+= 1;
    } else if (key == XK_Down) {
        locy += 1;
    } else if (key == XK_Up) {
        locy -= 1;
    }
    
    // Corrected call
    XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);
}

FarbfeldImage *load_farbfeld(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    char magic[8];
    if (fread(magic, 1, 8, f) != 8 || memcmp(magic, "farbfeld", 8) != 0) {
        fclose(f);
        return NULL;
    }

    uint8_t buf[4];
    fread(buf, 1, 4, f);
    uint32_t width = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
    fread(buf, 1, 4, f);
    uint32_t height = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];

    size_t pixels_size = width * height * 4 * 2;
    uint8_t *raw = malloc(pixels_size);
    if (!raw || fread(raw, 1, pixels_size, f) != pixels_size) {
        fclose(f);
        free(raw);
        return NULL;
    }

    fclose(f);

    FarbfeldImage *img = malloc(sizeof(FarbfeldImage));
    img->width = width;
    img->height = height;
    img->pixels = malloc(pixels_size);
    for (size_t i = 0; i < width * height * 4; ++i) {
        img->pixels[i] = (raw[2*i] << 8) | raw[2*i+1];
    }

    free(raw);
    return img;
}

void free_farbfeld(FarbfeldImage *img) {
    if (img) {
        free(img->pixels);
        free(img);
    }
}

void draw_image(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (!img) return;

    int draw_width = img->width * zoom;
    int draw_height = img->height * zoom;

    // Use 32bpp (4 bytes per pixel: R, G, B, pad)
    int bpp = 4;
    int stride = draw_width * bpp;

    unsigned char *rgbdata = malloc(draw_height * stride);
    if (!rgbdata) return;

    for (int y = 0; y < draw_height; ++y) {
        int sy = y / zoom + locx;
        if (sy >= (int)img->height) sy = img->height - 1;
        for (int x = 0 ; x < draw_width; ++x) {
            int sx = x / zoom +locy;
            if (sx >= (int)img->width) sx = img->width - 1;

            int src_idx = (sy * img->width + sx) * 4;
            uint8_t r = img->pixels[src_idx] >> 8;
            uint8_t g = img->pixels[src_idx + 1] >> 8;
            uint8_t b = img->pixels[src_idx + 2] >> 8;

            int dst_idx = y * stride + x * bpp;
            rgbdata[dst_idx + 0] = b;  // blue first
            rgbdata[dst_idx + 1] = g;
            rgbdata[dst_idx + 2] = r;
            //rgbdata[dst_idx + 0] = r;
            //rgbdata[dst_idx + 1] = g;
            //rgbdata[dst_idx + 2] = b;
            rgbdata[dst_idx + 3] = 0; // padding byte
        }
    }

    XImage *xim = XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
                               24, ZPixmap, 0, (char*)rgbdata,
                               draw_width, draw_height, 32, stride);

    XPutImage(dpy, win, gc, xim, 0, 0, 0, 0, draw_width, draw_height);
    XDestroyImage(xim); // also frees rgbdata
}
