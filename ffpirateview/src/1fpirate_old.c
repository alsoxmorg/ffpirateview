/* 1fpirate.c - 1bfarbf viewer with similar features to ffpirate*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_Render.h>
#include <arpa/inet.h>

#define OLDMAGIC "1bfarbf\0"
#define MAGIC "1bfarbfe"

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600
#define ZOOM_STEP 0.1f
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 10.0f

//float scalex = 1.0f;
//float scaley = 1.0f;
  

uint32_t *pixels = NULL;
uint32_t width = 0, height = 0;
uint32_t sw = 0, sh = 0;

SDL_Texture *texture = NULL;
SDL_Renderer *renderer = NULL;
SDL_Window *window = NULL;

int inverted_colors = 1;
uint8_t *bitmap = NULL;
int row_bytes; // = (width + 7) / 8;

void update_texture();


int get_pixel(int x, int y) {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  int byte_index = y * row_bytes + (x / 8);
  int bit_index = 7 - (x % 8);
  return (bitmap[byte_index] >> bit_index) & 1;
}
void set_pixel(int x, int y, int value) {
  if (x < 0 || x >= width || y < 0 || y >= height) return;
  int byte_index = y * row_bytes + (x / 8);
  int bit_index = 7 - (x % 8);
	if (value)
    bitmap[byte_index+1] |= (1 << bit_index);
  else
    bitmap[byte_index+1] &= ~(1 << bit_index);
}


void invert_colors() {
  //int row_bytes = (width + 7) / 8;
  uint32_t *new_pixels = malloc(width * height * sizeof(uint32_t));
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      /*      int byte_index = y * row_bytes + (x / 8);
      int bit_index = 7 - (x % 8);
      int bit = (bitmap[byte_index] >> bit_index) & 1;
      if (inverted_colors == 0)
	new_pixels[y * width + x] = bit ? 0x00000000 : 0xFFFFFFFF;
      //set_pixel(x,y, 0x00000000 : 0xFFFFFFFF);
      else
	new_pixels[y * width + x] = bit ? 0xFFFFFFFF : 0x00000000;
      //set_pixel(x,y, 0xFFFFFFFF : 0x00000000);
      */
      
      
      }
  }
  free(pixels);
  pixels = new_pixels;
  SDL_DestroyTexture(texture);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
			      SDL_TEXTUREACCESS_STREAMING, width, height);
  
  update_texture();
}


void update_texture() {
  SDL_Rect t;
  t.x = 0;
  t.y = 0;
  t.w = 800 + sw;
  t.y = 600 + sh;
  
  //SDL_RenderSetScale(renderer, scalex, scaley);
  SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}


/*void update_texture()
{
  uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int byte_index = y * row_bytes + (x / 8);
      int bit_index = 7 - (x % 8);
      int bit = (bitmap[byte_index] >> bit_index) & 1;
      pixels[y * width + x] = bit ? 0x00000000 : 0xFFFFFFFF;
    }
  }
  SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(uint32_t));
  free(pixels);
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer); 
  }*/

void rotate_image() {
  uint32_t *new_pixels = malloc(width * height * sizeof(uint32_t));
  for (uint32_t y = 0; y < height; ++y)
    for (uint32_t x = 0; x < width; ++x)
      new_pixels[x * height + (height - y - 1)] = pixels[y * width + x];
  free(pixels);
  pixels = new_pixels;
  uint32_t tmp = width;
  width = height;
  height = tmp;
  SDL_DestroyTexture(texture);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  update_texture();
}

void flip_horizontal() {
  for (uint32_t y = 0; y < height; ++y)
    for (uint32_t x = 0; x < width / 2; ++x) {
      uint32_t tmp = pixels[y * width + x];
      pixels[y * width + x] = pixels[y * width + (width - x - 1)];
      pixels[y * width + (width - x - 1)] = tmp;
    }
  update_texture();
}

void flip_vertical() {
  for (uint32_t y = 0; y < height / 2; ++y)
    for (uint32_t x = 0; x < width; ++x) {
      uint32_t tmp = pixels[y * width + x];
      pixels[y * width + x] = pixels[(height - y - 1) * width + x];
      pixels[(height - y - 1) * width + x] = tmp;
    }
  update_texture();
}

void save_as_1f(const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (!fp) return;
  fwrite(MAGIC, 1, 8, fp);
  uint32_t w = htonl(width), h = htonl(height);
  fwrite(&w, 4, 1, fp);
  fwrite(&h, 4, 1, fp);

  //int row_bytes = (width + 7) / 8;
  for (uint32_t y = 0; y < height; y++) {
    uint8_t row[row_bytes];
    memset(row, 0, row_bytes);
    for (uint32_t x = 0; x < width; x++) {
      uint32_t color = pixels[y * width + x];
      int bit = (color & 0xFFFFFF) > 0 ? 1 : 0;
      row[x / 8] |= (bit << (7 - (x % 8)));
    }
    fwrite(row, 1, row_bytes, fp);
  }
  fclose(fp);
  printf("Saved to %s\n", filename);
}


int main(int argc, char **argv) {
  //row_bytes = (width + 7) / 8;
  if (argc != 2) {
    fprintf(stderr, "Usage: %s file.1f\n", argv[0]);
    return 1;
  }

  FILE *fp = fopen(argv[1], "rb");
  if (!fp) {
    perror("fopen");
    return 1;
  }

  char magic[8];
  fread(magic, 1, 8, fp);
  if (memcmp(magic, MAGIC, 8) != 0 &&  memcmp(magic, OLDMAGIC, 8) != 0) {
    fprintf(stderr, "Invalid 1f format\n");
    fclose(fp);
    return 1;
  }

  fread(&width, 4, 1, fp);
  fread(&height, 4, 1, fp);
  width = ntohl(width);
  height = ntohl(height);

  row_bytes = (width + 7) / 8;
  //uint8_t *bitmap = malloc(row_bytes * height);
  bitmap = malloc(row_bytes * height);
  fread(bitmap, 1, row_bytes * height, fp);
  fclose(fp);

  pixels = malloc(width * height * sizeof(uint32_t));
  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      int byte_index = y * row_bytes + (x / 8);
      int bit_index = 7 - (x % 8);
      int bit = (bitmap[byte_index] >> bit_index) & 1;
      if(inverted_colors == 0)
	pixels[y * width + x] = bit ? 0x00000000 : 0xFFFFFFFF; //0xFFFFFFFF : 0x00000000;
      else
	pixels[y * width + x] = bit ? 0xFFFFFFFF : 0x00000000;
    }
  }
  free(bitmap);

  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("1f Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			    width, height, SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);

  update_texture();

  SDL_Event e;
  while (SDL_WaitEvent(&e)) {
    if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
      break;
    else if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_q))
      break;
    else if (e.type == SDL_KEYDOWN) {
      switch (e.key.keysym.sym) {
      case SDLK_r: rotate_image(); break;
      case SDLK_h: flip_horizontal(); break;
      case SDLK_v: flip_vertical(); break;
      case SDLK_s: save_as_1f("saved.1f"); break;
      case SDLK_i: invert_colors(); break;
	//case SDLK_1: scalex = 1.0f; scaley = 1.0f; break;
	//case SDLK_MINUS: scalex -= 0.1f; scaley -= 0.1f; break;
	//case SDLK_PLUS: scalex += 0.1f; scaley += 0.1f; break;
      case SDLK_MINUS: sw -= 10; sh -= 10; break;
      case SDLK_PLUS: sw += 10; sh += 10; break;
	//case SDLK_s: invert_colors(); break;
      }
    }
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  free(pixels);
  return 0;
}
