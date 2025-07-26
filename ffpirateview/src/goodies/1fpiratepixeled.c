#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define MAGIC "1fpirate"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s input.1ff x y bit > output.1ff\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    uint32_t x = atoi(argv[2]);
    uint32_t y = atoi(argv[3]);
    int bit = (argv[4][0] == '1') ? 1 : 0;

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    char magic[8];
    fread(magic, 1, 8, fp);
    if (memcmp(magic, MAGIC, 8) != 0) {
        fprintf(stderr, "Not a 1fpirate file\n");
        fclose(fp);
        return 1;
    }

    uint32_t width, height;
    fread(&width, 4, 1, fp);
    fread(&height, 4, 1, fp);
    width = ntohl(width);
    height = ntohl(height);

    size_t row_bytes = (width + 7) / 8;
    size_t size = row_bytes * height;
    uint8_t *bitmap = malloc(size);
    if (!bitmap) {
        perror("malloc");
        fclose(fp);
        return 1;
    }

    fread(bitmap, 1, size, fp);
    fclose(fp);

    if (x >= width || y >= height) {
        fprintf(stderr, "Pixel out of bounds! Image is %ux%u\n", width, height);
        free(bitmap);
        return 1;
    }

    // Edit the pixel
    size_t byte_index = y * row_bytes + (x / 8);
    int bit_index = 7 - (x % 8);

    if (bit)
        bitmap[byte_index] |= (1 << bit_index);
    else
        bitmap[byte_index] &= ~(1 << bit_index);

    // Write output to stdout
    fwrite(MAGIC, 1, 8, stdout);
    uint32_t w_be = htonl(width);
    uint32_t h_be = htonl(height);
    fwrite(&w_be, 4, 1, stdout);
    fwrite(&h_be, 4, 1, stdout);
    fwrite(bitmap, 1, size, stdout);

    free(bitmap);
    return 0;
}
/*literally gimp*/
