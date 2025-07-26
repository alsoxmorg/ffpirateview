#define SDL_FARBFELD_IMPLEMENTATION
#include "SDL_Farbfeld.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

/*
SDL_Surface *SDL_LoadFarbfeldSurface(const char *filename) {
    uint32_t width, height;
    SDL_Surface *surf;
    uint32_t rmask, gmask, bmask, amask;
*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <image.ff>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    SDL_Surface *surface = SDL_LoadFarbfeldSurface(filename);
    if (!surface) {
        fprintf(stderr, "Failed to load image: %s\n", filename);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Farbfeld Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
        surface->w, surface->h);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTexture error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_FreeSurface(surface);
        SDL_Quit();
        return 1;
    }

    SDL_UpdateTexture(texture, NULL, surface->pixels, surface->w * 4);

    float zoom = 1.0f;
    int quit = 0;

    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = 1;
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        quit = 1;
                        break;
                    case SDLK_EQUALS:
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        zoom *= 1.1f;
                        break;
                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        zoom /= 1.1f;
                        if (zoom < 0.1f) zoom = 0.1f;
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        int tex_w = (int)(surface->w * zoom);
        int tex_h = (int)(surface->h * zoom);
        int win_w, win_h;
        SDL_GetWindowSize(window, &win_w, &win_h);

        SDL_Rect dst_rect = {
            .x = (win_w - tex_w) / 2,
            .y = (win_h - tex_h) / 2,
            .w = tex_w,
            .h = tex_h
        };

        SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_FreeSurface(surface);
    SDL_Quit();
    return 0;
}
