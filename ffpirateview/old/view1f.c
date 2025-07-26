#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <arpa/inet.h>

#define MAGIC "1bfarbf\0"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s image.1f\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    char magic[8];
    fread(magic, 1, 8, fp);
    if (memcmp(magic, MAGIC, 8) != 0) {
        fprintf(stderr, "Not a valid 1bfarb file.\n");
        fclose(fp);
        return 1;
    }

    uint32_t width, height;
    fread(&width, 4, 1, fp);
    fread(&height, 4, 1, fp);
    width = ntohl(width);
    height = ntohl(height);

    int row_bytes = (width + 7) / 8;
    uint8_t *bitmap = malloc(row_bytes * height);
    fread(bitmap, 1, row_bytes * height, fp);
    fclose(fp);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
        "1f Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int byte_index = y * row_bytes + (x / 8);
            int bit_index = 7 - (x % 8);
            int bit = (bitmap[byte_index] >> bit_index) & 1;
            pixels[y * width + x] = bit ? 0xFFFFFFFF : 0xFF000000; // white or black
        }
    }

    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // Wait for quit
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
            break;
        }
    }

    free(bitmap);
    free(pixels);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
