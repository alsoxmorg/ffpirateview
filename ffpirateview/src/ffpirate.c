/* ffpirate.c views a farbfeld image in an SDL window*/
#define SDL_FARBFELD_IMPLEMENTATION
#include "SDL_Farbfeld.h"

#ifdef SDL3
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define    MAGIC "1fpirate"
#define OLDMAGIC "1bfarbfe"
#define  FFMAGIC "farbfeld"
#define MODE_FF 1
#define MODE_1F 0

#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 800
#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

#define ZOOM_STEP 0.1f
#define MIN_ZOOM 0.1f
#define MAX_ZOOM 10.0f

int is_fullscreen = 0;
int show_saved_bar = 0;
Uint32 saved_bar_start = 0;
const Uint32 saved_bar_duration = 2000;  // 2 seconds
FILE *fp = NULL;
//uint32_t width, height;
uint8_t *bitmap = NULL;
int width = 0, height = 0, row_bytes = 0;
int image_mode = MODE_FF; /*MODE_1F, MODE_FF*/
SDL_Renderer *renderer;

float zoom = 1.0f;
int panX = 0;
int panY = 0;
const int frameDelay = 1000 / 30;  // cap to 30 FPS


int test_magic(FILE *fp) {
  /*FILE *fp = fopen(argv, "rb");*/
  if (!fp) {
    perror("Failed to open file");
    return -1;
  }
  uint8_t hdr[8];  // Only need 8 bytes for magic
  if (fread(hdr, 1, 8, fp) != 8) {
    fclose(fp);
    return -1;
  }
  //fclose(fp);
  if (memcmp(hdr, "farbfeld", 8) == 0) {
    printf("test_magic: It's a farbfeld\n");
    image_mode = MODE_FF;
    return 1;
  } else if (memcmp(hdr, "1fpirate", 8) == 0) {
    printf("test_magic: It's a 1b farbfeld\n");
    image_mode = MODE_1F;
    return 2;
  } else {
    printf("test_magic: Unknown format\n");
    return 3;
  }
} /** test magic **/
SDL_Surface *SDL_LoadFarbfeldSurfaceFromFP(FILE *fp);

int get_pixel(int x, int y) {
  if (x < 0 || x >= width || y < 0 || y >= height) return 0;
  int byte_index = y * row_bytes + (x / 8);
  int bit_index = 7 - (x % 8);
  return (bitmap[byte_index] >> bit_index) & 1;
}

SDL_Surface *load_1f_fromFP(FILE *fp)
{
    rewind(fp); // Start at beginning
    printf("DEBUG: Using load_1f_fromFP\n");

    char magic[9] = {0};
    if (fread(magic, 1, 8, fp) != 8) return NULL;
    if (memcmp(magic, MAGIC, 8) != 0) {
        fprintf(stderr, "Invalid magic\n");
        return NULL;
    }

    uint32_t w_be, h_be;
    if (fread(&w_be, 4, 1, fp) != 1) return NULL;
    if (fread(&h_be, 4, 1, fp) != 1) return NULL;

    width = ntohl(w_be);
    height = ntohl(h_be);
    row_bytes = (width + 7) / 8;

    bitmap = malloc(row_bytes * height);
    if (!bitmap) return NULL;

    if (fread(bitmap, 1, row_bytes * height, fp) != row_bytes * height) {
        free(bitmap);
        return NULL;
    }

    // Allocate pixel buffer for SDL
    uint32_t *pixels = malloc(width * height * sizeof(uint32_t));
    if (!pixels) {
        free(bitmap);
        return NULL;
    }

    // Fill in pixels based on bitmap bits
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            int byte_index = y * row_bytes + (x / 8);
            int bit_index = 7 - (x % 8);
            int bit = (bitmap[byte_index] >> bit_index) & 1;
            pixels[y * width + x] = bit ? 0xFFFFFFFF : 0x00000000;
        }
    }

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
        pixels,
        width,
        height,
        32,                   // bits per pixel
        width * 4,            // pitch (row stride)
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif
    );

    if (!surface) {
        free(pixels);
        free(bitmap);
        return NULL;
    }

    // Make sure SDL frees the pixel buffer when done
    surface->flags &= ~SDL_PREALLOC;
    printf("load_1f_fromFP: retuirning surface\n");
    return surface;
}

///////////////////////////////////////////////////////////////////
SDL_Surface * load_pirate(FILE *f, int t) /*file pointer, type*/
{ /* load a 1fpirate, or a farbfeld*/
  if (!f) return 0;
  /*    image_mode = MODE_FF; MODE_1F, MODE_FF*/
  if (image_mode == MODE_1F)
    {
      SDL_Surface *s = load_1f_fromFP(f);
      return s;
    }
  else if (image_mode == MODE_FF){
    SDL_Surface *s = SDL_LoadFarbfeldSurfaceFromFP(f); //SDL_LoadFarbeldSurfaceFromFP(f);
    return s;
  } /*we checked the magic for farbfeld*/
  else /*invalid magic*/
    {
      printf("load_pirate : error invalid magic.\n");
      fclose(f);  /*image_mode = MODE_1F; MODE_1F, MODE_FF*/
      return NULL;
    }
}

void update_texture(SDL_Texture *texture) {
  /*void render_image() {
  SDL_RenderClear(renderer);
  SDL_Rect dest = {
    .x = offset_x,
    .y = offset_y,
    .w = (int)(width * zoom),
    .h = (int)(height * zoom)
  };
  SDL_RenderCopy(renderer, texture, NULL, &dest);
  SDL_RenderPresent(renderer);
}*/

  SDL_Rect dest = {
    .x = panX,
    .y = panY,
    .w = (int)(width * zoom),
    .h = (int)(height * zoom)
  };
  if(image_mode == MODE_1F) {
    SDL_RenderClear(renderer);
    //SDL_RenderSetScale(renderer, zoom, zoom);
    SDL_RenderCopy(renderer, texture, NULL, &dest); //&dstRect);
    SDL_RenderPresent(renderer);
  }
  else if(image_mode == MODE_FF) {
    SDL_RenderClear(renderer);
    //SDL_Rect dstRect = { panX, panY, 0, 0 };
    SDL_RenderSetScale(renderer, zoom, zoom);
    SDL_RenderCopy(renderer, texture, NULL, &dest); //&dstRect);
    SDL_RenderPresent(renderer);
  }
} /*update_texture*/

SDL_Surface *SDL_LoadFarbfeldSurfaceFromFP(FILE *fp) {
  if(!fp) {
    printf("from: SDL_LoadFarbfeldSurfaceFromFP: your file is not open...die.\n");
    return NULL;
  }
  printf("SDL_LoadFarbfeldSurfaceFromFP: fread width/height\n");
  fread(&width, 4, 1, fp);
  fread(&height, 4, 1, fp);
  width = ntohl(width);
  height = ntohl(height);
  printf("SDL_LoadFarbfeldSurfaceFromFP: width=%u, height=%u\n", width, height);
  SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
  if (!surf) return NULL;
  
  uint8_t *dst = surf->pixels;
  for (uint32_t i = 0; i < width * height; i++) {
    uint16_t r, g, b, a;
    fread(&r, 2, 1, fp); r = ntohs(r);
    fread(&g, 2, 1, fp); g = ntohs(g);
    fread(&b, 2, 1, fp); b = ntohs(b);
    fread(&a, 2, 1, fp); a = ntohs(a);
    
    dst[i * 4 + 0] = r >> 8;
    dst[i * 4 + 1] = g >> 8;
    dst[i * 4 + 2] = b >> 8;
    dst[i * 4 + 3] = a >> 8;
  }
  return surf;
} /*SDL_LoadFarbfeldSurfaceFromFP*/


void SDL_SaveFarbfeldSurface(SDL_Surface *surface, const char *filename) {
  if (!surface || !filename) {
    fprintf(stderr, "Invalid arguments to SDL_SaveFarbfeldSurface\n");
    return;
  }
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    perror("fopen");
    return;
  }

  // Write "farbfeld" magic
  fwrite("farbfeld", 1, 8, fp);

  // Write width and height in big-endian
  uint32_t width = surface->w;
  uint32_t height = surface->h;

  for (int shift = 24; shift >= 0; shift -= 8)
    fputc((width >> shift) & 0xFF, fp);
  for (int shift = 24; shift >= 0; shift -= 8)
    fputc((height >> shift) & 0xFF, fp);

  // Convert and write pixel data: 4x 16-bit BE per pixel
  SDL_LockSurface(surface);

  for (int y = 0; y < height; y++) {
    uint32_t *row = (uint32_t *)((uint8_t *)surface->pixels + y * surface->pitch);
    for (int x = 0; x < width; x++) {
      uint8_t r, g, b, a;
      SDL_GetRGBA(row[x], surface->format, &r, &g, &b, &a);
      uint16_t r16 = r << 8;
      uint16_t g16 = g << 8;
      uint16_t b16 = b << 8;
      uint16_t a16 = a << 8;
      uint16_t components[4] = {r16, g16, b16, a16};
      for (int i = 0; i < 4; i++) {
	fputc(components[i] >> 8, fp);
	fputc(components[i] & 0xFF, fp);
      }
    }
  }
  SDL_UnlockSurface(surface);
  fclose(fp);
} /*SDL_SaveFarbfeldSurface(SDL_Surface *surface, const char *filename)*/

void SaveFarbfeldSurface(SDL_Surface *surface, const char *filename) {
  char newname[1024];
  snprintf(newname, sizeof(newname), "%s.new.ff", filename);
  SDL_SaveFarbfeldSurface(surface, newname);
  printf("Saved to %s\n", newname);
} /*SaveFarbfeldSurface*/

SDL_Surface *RotateSurface90(SDL_Surface *src) {
  SDL_Surface *rotated = SDL_CreateRGBSurfaceWithFormat(0, src->h, src->w, 32, src->format->format);
  if (!rotated) return NULL;

  SDL_LockSurface(src);
  SDL_LockSurface(rotated);

  Uint32 *srcPixels = (Uint32 *)src->pixels;
  Uint32 *dstPixels = (Uint32 *)rotated->pixels;
  for (int y = 0; y < src->h; y++) {
    for (int x = 0; x < src->w; x++) {
      dstPixels[x * rotated->w + (rotated->w - y - 1)] = srcPixels[y * src->w + x];
    }
  }
  SDL_UnlockSurface(src);
  SDL_UnlockSurface(rotated);
  return rotated;
} /*SDL_Surface *RotateSurface90*/

SDL_Surface *FlipSurface(SDL_Surface *src, int horizontal) {
  SDL_Surface *flipped = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, src->format->format);
  if (!flipped) return NULL;
  SDL_LockSurface(src);
  SDL_LockSurface(flipped);
  Uint32 *srcPixels = (Uint32 *)src->pixels;
  Uint32 *dstPixels = (Uint32 *)flipped->pixels;
  for (int y = 0; y < src->h; y++) {
    for (int x = 0; x < src->w; x++) {
      int dstX = horizontal ? (src->w - x - 1) : x;
      int dstY = horizontal ? y : (src->h - y - 1);
      dstPixels[dstY * src->w + dstX] = srcPixels[y * src->w + x];
    }
  }

  SDL_UnlockSurface(src);
  SDL_UnlockSurface(flipped);
  return flipped;
} /*FlipSurface*/


#include <unistd.h>

int main(int argc, char *argv[]) {
  FILE *fp = NULL;  
  if (argc > 1) {
    fp = fopen(argv[1], "rb");
    if (!fp) {
      perror("Failed to open file");
      return 1;
    }
  } else if (!isatty(fileno(stdin))) {
    fp = stdin;
  } else {
    fprintf(stderr, "Usage: %s <file.ff>\nOr pipe farbfeld data via stdin\n", argv[0]);
    return 1;
  } /*SDL_Init(SDL_INIT_VIDEO & ~SDL_INIT_AUDIO);*/
  if (SDL_Init(SDL_INIT_VIDEO & ~SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  if(!fp) printf("file not open after =stdin from arg....die\n");
  SDL_Window *window = SDL_CreateWindow("ffPirateView",
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
					SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  //SDL_Renderer *
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  if(test_magic(fp) == 3)
    {
      printf("its an unknown format! bailing!\n");
      exit(2);
    }
  
  if(!fp)
    printf("file not open after test_magic....die\n");
  else
    printf("main: file appears to be open.... live?\n");
  //SDL_Surface *surface = SDL_LoadFarbfeldSurface(argv[1]);
  //SDL_Surface *surface = SDL_LoadFarbfeldSurfaceFromFP(fp);
  SDL_Surface *surface  = load_pirate(fp,image_mode); 
  if (!surface) {
    fprintf(stderr, "Could not load farbfeld image(null serface): %s\n", SDL_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  printf("main: SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);\n");
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (!texture) {
    fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  int quit = 0;
  SDL_Event event;
  //float zoom = 1.0f;
  //int panX = 0, panY = 0;
  int dragging = 0;
  int lastMouseX = 0, lastMouseY = 0;

  while (!quit) {
    Uint32 frameStart = SDL_GetTicks();
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
	case SDLK_q:
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
	  //SDL_RenderSetScale(renderer, zoom, zoom);
	  break;
	  /*
case SDLK_PLUS:
        case SDLK_EQUALS:
          zoom += ZOOM_STEP;
          if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
          break;
        case SDLK_MINUS:
          zoom -= ZOOM_STEP;
          if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
          break;
	   */

	  
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
	  zoom -= ZOOM_STEP;
	  if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
	  //SDL_RenderSetScale(renderer, zoom, zoom);
	  break;
	case SDLK_r: {
	  SDL_Surface *rot = RotateSurface90(surface);
	  if (rot) {
	    SDL_FreeSurface(surface);
	    SDL_DestroyTexture(texture);
	    surface = rot;
	    texture = SDL_CreateTextureFromSurface(renderer, surface);
	  }
	  break;
	}
	case SDLK_f:
	  is_fullscreen = !is_fullscreen;
	  if (is_fullscreen) {
	    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	  } else {
	    //SDL_SetWindowFullscreen(window, 0);
	    SDL_SetWindowFullscreen(window, 0);
	    SDL_SetWindowSize(window, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
	    SDL_SetWindowPosition(window,
				  INITIAL_WINDOW_WIDTH,
				  INITIAL_WINDOW_HEIGHT);
	  }
	  break;
	case SDLK_h:
	case SDLK_v: {
	  int horiz = event.key.keysym.sym == SDLK_h;
	  SDL_Surface *flp = FlipSurface(surface, horiz);
	  if (flp) {
	    SDL_FreeSurface(surface);
	    SDL_DestroyTexture(texture);
	    surface = flp;
	    texture = SDL_CreateTextureFromSurface(renderer, surface);
	  }
	  break;
	}
	case SDLK_s:
	  show_saved_bar = 1;
	  saved_bar_start = SDL_GetTicks();
	  SaveFarbfeldSurface(surface, argv[1]);
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
    update_texture(texture);
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < frameDelay) {
      SDL_Delay(frameDelay - frameTime);
    }
  }

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
