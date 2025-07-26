#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC "1bfarbf\0"
#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 800
#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600


#define ZOOM_STEP 0.1f
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 10.0f


int is_fullscreen = 0;
uint8_t *bitmap = NULL;
int width = 0, height = 0, row_bytes = 0;
SDL_Texture *texture;
SDL_Renderer *renderer;

void update_texture(SDL_Texture *texture) {
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int byte_index = y * row_bytes + (x / 8);
            int bit_index = 7 - (x % 8);
            int bit = (bitmap[byte_index] >> bit_index) & 1;
            pixels[y * width + x] = bit ? 0xFFFFFFFF : 0x00000000;
        }
    }
    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
    free(pixels);
}

void save_1f(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Save failed");
        return;
    }
    fwrite(MAGIC, 1, 8, f);

    uint32_t w_be = htonl(width);
    uint32_t h_be = htonl(height);
    fwrite(&w_be, 4, 1, f);
    fwrite(&h_be, 4, 1, f);
    fwrite(bitmap, 1, row_bytes * height, f);
    fclose(f);
    printf("Saved to %s\n", filename);
}

void set_pixel(int x, int y, int value) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int byte_index = y * row_bytes + (x / 8);
    int bit_index = 7 - (x % 8);
    if (value)
        bitmap[byte_index] |= (1 << bit_index);
    else
        bitmap[byte_index] &= ~(1 << bit_index);
}

int get_pixel(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    int byte_index = y * row_bytes + (x / 8);
    int bit_index = 7 - (x % 8);
    return (bitmap[byte_index] >> bit_index) & 1;
}

void flip_horizontal() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 2; x++) {
            int tmp = get_pixel(x, y);
            set_pixel(x, y, get_pixel(width - 1 - x, y));
            set_pixel(width - 1 - x, y, tmp);
        }
    }
}

void flip_vertical() {
    for (int y = 0; y < height / 2; y++) {
        for (int x = 0; x < width; x++) {
            int tmp = get_pixel(x, y);
            set_pixel(x, y, get_pixel(x, height - 1 - y));
            set_pixel(x, height - 1 - y, tmp);
        }
    }
}

void rotate_90() {
    int new_w = height;
    int new_h = width;
    int new_row = (new_w + 7) / 8;
    uint8_t *new_bmp = calloc(new_row * new_h, 1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int val = get_pixel(x, y);
            int nx = y;
            int ny = width - 1 - x;
            int idx = ny * new_row + (nx / 8);
            int bidx = 7 - (nx % 8);
            if (val) new_bmp[idx] |= (1 << bidx);
        }
    }

    free(bitmap);
    bitmap = new_bmp;
    width = new_w;
    height = new_h;
    row_bytes = new_row;

    SDL_DestroyTexture(texture);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, width, height);
    update_texture(texture);
}

int load_1f(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;

    char magic[9] = {0};
    fread(magic, 1, 8, f);
    if (memcmp(magic, MAGIC, 8) != 0) {
        fclose(f);
        return 0;
    }

    uint32_t w_be, h_be;
    fread(&w_be, 4, 1, f);
    fread(&h_be, 4, 1, f);
    width = ntohl(w_be);
    height = ntohl(h_be);
    row_bytes = (width + 7) / 8;

    bitmap = malloc(row_bytes * height);
    fread(bitmap, 1, row_bytes * height, f);
    fclose(f);
    return 1;
}

void blank_canvas() {
    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;
    row_bytes = (width + 7) / 8;
    bitmap = calloc(row_bytes * height, 1); // starts as white (0)
}

int main(int argc, char *argv[]) {
    const char *input = argc >= 2 ? argv[1] : NULL;
    const char *outfile = argc >= 3 ? argv[2] : "output.1ff";

    if (input && !load_1f(input)) {
        fprintf(stderr, "Failed to load %s\n", input);
        return 1;
    }

    if (!input) blank_canvas();

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("1ffedit", SDL_WINDOWPOS_CENTERED,
					  SDL_WINDOWPOS_CENTERED, width, height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
				SDL_TEXTUREACCESS_STREAMING, width, height);

    update_texture(texture);
    int quit = 0;
    SDL_Event e;
    float zoom = 1.0f;
    int panX = 0, panY = 0;
    int dragging = 0;
    
    int left_down = 0, right_down = 0;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = 1;
            else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;
                if (key == SDLK_q || key == SDLK_ESCAPE) quit = 1;
                else if (key == SDLK_s) save_1f(outfile);
                else if (key == SDLK_r) rotate_90();
                else if (key == SDLK_h) flip_horizontal();
                else if (key == SDLK_v) flip_vertical();
		else if (key == SDLK_f) {
		  is_fullscreen = !is_fullscreen;
		  if (is_fullscreen) {
		    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		  } else {
		    //SDL_SetWindowFullscreen(window, 0);
		    SDL_SetWindowFullscreen(window, 0);
		    SDL_SetWindowSize(window, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
		    SDL_SetWindowPosition(window, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
		    //#define INITIAL_WINDOW_WIDTH 800
		    //#define INITIAL_WINDOW_HEIGHT 600
		  }
		}

		
		else if (key == SDLK_KP_PLUS || key == SDLK_EQUALS)
		  {
		    zoom += ZOOM_STEP;
		    if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
		    SDL_RenderSetScale(renderer, zoom, zoom);
		  }
		else if(key ==  SDLK_MINUS || key == SDLK_KP_MINUS)
		  {
		    zoom -= ZOOM_STEP;
		    if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
		    SDL_RenderSetScale(renderer, zoom, zoom);
		  }
                update_texture(texture);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) left_down = 1;
                if (e.button.button == SDL_BUTTON_RIGHT) right_down = 1;
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) left_down = 0;
                if (e.button.button == SDL_BUTTON_RIGHT) right_down = 0;
            }
            else if (e.type == SDL_MOUSEMOTION) {
                int x = e.motion.x;
                int y = e.motion.y;
                if (left_down) {
                    set_pixel(x, y, 1); // draw black
                    update_texture(texture);
                } else if (right_down) {
                    set_pixel(x, y, 0); // draw white
                    update_texture(texture);
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(bitmap);
    return 0;
}
