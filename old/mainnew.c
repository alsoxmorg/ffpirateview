#define SDL_FARBFELD_IMPLEMENTATION
#include "SDL_Farbfeld.h"

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600
#define ZOOM_STEP 0.1f
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 10.0f

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <image.ff>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("farbfeld viewer",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Surface *surface = SDL_LoadFarbfeldSurface(argv[1]);
    if (!surface) {
        fprintf(stderr, "Could not load farbfeld image: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int quit = 0;
    SDL_Event event;
    float zoom = 1.0f;
    int panX = 0, panY = 0;
    int dragging = 0;
    int lastMouseX = 0, lastMouseY = 0;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            quit = 1;
                            break;
                        case SDLK_UP:
                            panY += 20;
                            break;
                        case SDLK_DOWN:
                            panY -= 20;
                            break;
                        case SDLK_LEFT:
                            panX += 20;
                            break;
                        case SDLK_RIGHT:
                            panX -= 20;
                            break;
                        case SDLK_EQUALS:
                        case SDLK_KP_PLUS:
                            zoom += ZOOM_STEP;
                            if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
                            SDL_RenderSetScale(renderer, zoom, zoom);
                            break;
                        case SDLK_MINUS:
                        case SDLK_KP_MINUS:
                            zoom -= ZOOM_STEP;
                            if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
                            SDL_RenderSetScale(renderer, zoom, zoom);
                            break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging = 1;
                        lastMouseX = event.button.x;
                        lastMouseY = event.button.y;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging = 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (dragging) {
                        panX += event.motion.x - lastMouseX;
                        panY += event.motion.y - lastMouseY;
                        lastMouseX = event.motion.x;
                        lastMouseY = event.motion.y;
                    }
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect dstRect = { panX, panY, 0, 0 };
        SDL_QueryTexture(texture, NULL, NULL, &dstRect.w, &dstRect.h);
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


