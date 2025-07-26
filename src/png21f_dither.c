#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <png.h>

#define MAGIC "1bfarbfe"

void write_uint32_be(FILE *f, uint32_t val) {
  fputc((val >> 24) & 0xFF, f);
  fputc((val >> 16) & 0xFF, f);
  fputc((val >> 8) & 0xFF, f);
  fputc(val & 0xFF, f);
}

int main(int argc, char *argv[]) {
  int dither = 1;
  if (argc < 3 || argc > 4) {
    fprintf(stderr, "Usage: %s [-d] input.png output.1f\n", argv[0]);
    return 1;
  }

  const char *input_file = NULL;
  const char *output_file = NULL;

  if (argc == 4 && strcmp(argv[1], "-d") == 0) {
    dither = 0;
    input_file = argv[2];
    output_file = argv[3];
  } else {
    input_file = argv[1];
    output_file = argv[2];
  }

  FILE *fp = fopen(input_file, "rb");
  if (!fp) {
    perror("fopen input");
    return 1;
  }

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  if (setjmp(png_jmpbuf(png))) {
    fprintf(stderr, "libpng read error\n");
    return 1;
  }

  png_init_io(png, fp);
  png_read_info(png, info);

  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  png_byte color_type = png_get_color_type(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);

  // Convert to 8-bit grayscale
  if (bit_depth == 16) png_set_strip_16(png);
  if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA)
    png_set_rgb_to_gray_fixed(png, 1, -1, -1);
  if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
  png_set_expand_gray_1_2_4_to_8(png);
  png_set_gray_to_rgb(png);
  png_read_update_info(png, info);

  png_bytep *rows = malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++) {
    rows[y] = malloc(png_get_rowbytes(png, info));
  }
  png_read_image(png, rows);
  fclose(fp);

  // Grayscale buffer
  float *gray = malloc(width * height * sizeof(float));
  for (int y = 0; y < height; y++) {
    png_bytep row = rows[y];
    for (int x = 0; x < width; x++) {
      png_bytep px = &(row[x * 3]);
      gray[y * width + x] = 0.299 * px[0] + 0.587 * px[1] + 0.114 * px[2];
    }
  }

  // Floyd–Steinberg dithering
  if (dither) {
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
	float old = gray[y * width + x];
	float new = old < 128 ? 0 : 255;
	float err = old - new;
	gray[y * width + x] = new;
	if (x + 1 < width)
	  gray[y * width + (x + 1)] += err * 7 / 16.0;
	if (y + 1 < height) {
	  if (x > 0)
	    gray[(y + 1) * width + (x - 1)] += err * 3 / 16.0;
	  gray[(y + 1) * width + x] += err * 5 / 16.0;
	  if (x + 1 < width)
	    gray[(y + 1) * width + (x + 1)] += err * 1 / 16.0;
	}
      }
    }
  }

  FILE *out = fopen(output_file, "wb");
  if (!out) {
    perror("fopen output");
    return 1;
  }

  fwrite(MAGIC, 1, 8, out);
  write_uint32_be(out, width);
  write_uint32_be(out, height);

  // Write bits
  for (int y = 0; y < height; y++) {
    for (int bx = 0; bx < (width + 7) / 8; bx++) {
      uint8_t byte = 0;
      for (int bit = 0; bit < 8; bit++) {
	int x = bx * 8 + bit;
	if (x < width) {
	  float v = gray[y * width + x];
	  byte |= ((v >= 128) ? 1 : 0) << (7 - bit);
	}
      }
      fputc(byte, out);
    }
  }

  fclose(out);
  for (int y = 0; y < height; y++) free(rows[y]);
  free(rows);
  free(gray);
  png_destroy_read_struct(&png, &info, NULL);
  return 0;
}
