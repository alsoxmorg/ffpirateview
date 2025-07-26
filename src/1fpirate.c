/* 1fpirate.c - 1bfarbf viewer with similar features to ffpirate*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <arpa/inet.h>

//#define OLDMAGIC "1bfarbf\0"
//#define MAGIC "1bfarbfe"

#define    MAGIC "1fpirate"
#define OLDMAGIC "1bfarbfe"


#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600
#define ZOOM_STEP 0.1f
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 10.0f

uint32_t *pixels = NULL;
uint32_t width = 0, height = 0;
uint32_t sw = 0, sh = 0;

SDL_Texture *texture = NULL;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

float zoom = 1.0f;
int offset_x = 0;
int offset_y = 0;

uint8_t *bitmap = NULL;
int row_bytes;

void update_texture();

int get_pixel(int x, int y) {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  int byte_index = y * row_bytes + (x / 8);
  int bit_index = 7 - (x % 8);
  return (bitmap[byte_index] >> bit_index) & 1;
}

void update_texture() {
  SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
}

void render_image() {
  SDL_RenderClear(renderer);
  SDL_Rect dest = {
    .x = offset_x,
    .y = offset_y,
    .w = (int)(width * zoom),
    .h = (int)(height * zoom)
  };
  SDL_RenderCopy(renderer, texture, NULL, &dest);
  SDL_RenderPresent(renderer);
}

#include <unistd.h>

int main(int argc, char *argv[]) {
  FILE *fp = NULL;

  if (argc > 1) {
    fp = fopen(argv[1], "rb");
    if (!fp) {
      perror("fopen");
      return 1;
    }
  } else if (!isatty(fileno(stdin))) {
    // stdin is being piped in
    fp = stdin;
  } else {
    fprintf(stderr, "Usage: %s <file.1f>\nOr pipe data via stdin\n", argv[0]);
    return 1;
  }

  char magic[8];
  fread(magic, 1, 8, fp);
  /*if (memcmp(magic, MAGIC, 8) != 0 && memcmp(magic, OLDMAGIC, 8) != 0) {
    fprintf(stderr, "Not a valid 1bfarbf file\n");
    return 1;
    }*/
  if (memcmp(magic, MAGIC, 8) != 0) {
    if (memcmp(magic, OLDMAGIC, 8) != 0) {
      fprintf(stderr, "%s: Invalid 1f file magic (%s)\n", argv[0], magic);
      fclose(fp);
      return 1;
    }
  }


  fread(&width, 4, 1, fp);
  fread(&height, 4, 1, fp);
  width = ntohl(width);
  height = ntohl(height);
  row_bytes = (width + 7) / 8;

  bitmap = malloc(row_bytes * height);
  fread(bitmap, 1, row_bytes * height, fp);
  fclose(fp);

  pixels = malloc(width * height * sizeof(uint32_t));
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      int bit = get_pixel(x, y);
      pixels[y * width + x] = bit ? 0xFFFFFFFF : 0x00000000;
    }
  }

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window =
    SDL_CreateWindow(
                     "1fpirate",
                     0,
                     0,
                     INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                     SDL_WINDOW_SHOWN |
                     SDL_WINDOW_RESIZABLE |
                     SDL_WINDOW_MAXIMIZED
                     );


  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  update_texture();

  SDL_Event e;
  int running = 1;
  while (running) {
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        running = 0;
      }
      else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
	case SDLK_q:
	  running = 0;
	  break;
	case SDLK_PLUS:
	case SDLK_EQUALS:
	  zoom += ZOOM_STEP;
	  if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
	  break;
	case SDLK_MINUS:
	  zoom -= ZOOM_STEP;
	  if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
	  break;
        }
      } else if (e.type == SDL_MOUSEMOTION && (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        offset_x += e.motion.xrel;
        offset_y += e.motion.yrel;
      }
    }
    render_image();
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  free(bitmap);
  free(pixels);

  return 0;
}
