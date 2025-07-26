#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define MAGIC "1fpirate"

int clamp(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

int main() {
    char magic[8];
    if (fread(magic, 1, 8, stdin) != 8 || memcmp(magic, "farbfeld", 8) != 0) {
        fprintf(stderr, "not a farbfeld file\n");
        return 1;
    }

    uint32_t width, height;
    fread(&width, 4, 1, stdin);
    fread(&height, 4, 1, stdin);
    width = ntohl(width);
    height = ntohl(height);

    int npixels = width * height;
    uint16_t *rgba = malloc(npixels * 8); // 4 * uint16_t per pixel
    if (!rgba) return 1;

    fread(rgba, 2, 4 * npixels, stdin);

    uint8_t *gray = malloc(npixels);
    int *error = calloc(npixels, sizeof(int));
    if (!gray || !error) return 1;

    // Convert to grayscale
    for (int i = 0; i < npixels; i++) {
        int r = ntohs(rgba[i*4 + 0]) >> 8;
        int g = ntohs(rgba[i*4 + 1]) >> 8;
        int b = ntohs(rgba[i*4 + 2]) >> 8;
        gray[i] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
    }

    // Apply Floyd–Steinberg dithering
    uint8_t *bits = calloc(((width + 7) / 8) * height, 1);
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            int i = y * width + x;
            int old = gray[i] + error[i];
            int new = old > 127 ? 255 : 0;
            int quant_err = old - new;
            gray[i] = new;

            // Set bit
            if (new == 255) {
                size_t byte = y * ((width + 7) / 8) + (x / 8);
                bits[byte] |= (1 << (7 - (x % 8)));
            }

            if (x + 1 < width) error[i + 1] += quant_err * 7 / 16;
            if (x > 0 && y + 1 < height) error[i + width - 1] += quant_err * 3 / 16;
            if (y + 1 < height) error[i + width] += quant_err * 5 / 16;
            if (x + 1 < width && y + 1 < height) error[i + width + 1] += quant_err * 1 / 16;
        }
    }

    // Write 1fpirate output
    fwrite(MAGIC, 1, 8, stdout);
    uint32_t w = htonl(width), h = htonl(height);
    fwrite(&w, 4, 1, stdout);
    fwrite(&h, 4, 1, stdout);
    fwrite(bits, 1, ((width + 7) / 8) * height, stdout);

    free(rgba);
    free(gray);
    free(error);
    free(bits);
    return 0;
}

