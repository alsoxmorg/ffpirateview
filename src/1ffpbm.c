#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAGIC "1bfarbfe"

uint32_t read_be32(FILE *f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

void write_be32(FILE *f, uint32_t val) {
    fputc((val >> 24) & 0xFF, f);
    fputc((val >> 16) & 0xFF, f);
    fputc((val >> 8) & 0xFF, f);
    fputc(val & 0xFF, f);
}

void convert_1ff_to_pbm(FILE *in) {
    char magic[9] = {0};
    fread(magic, 1, 8, in);
    if (memcmp(magic, MAGIC, 8) != 0) {
        fprintf(stderr, "Not a valid 1ff file\n");
        exit(1);
    }

    uint32_t w = read_be32(in);
    uint32_t h = read_be32(in);
    int row_bytes = (w + 7) / 8;
    uint8_t *data = malloc(h * row_bytes);
    fread(data, 1, h * row_bytes, in);

    printf("P1\n%d %d\n", w, h);
    for (uint32_t y = 0; y < h; y++) {
        for (uint32_t x = 0; x < w; x++) {
            int byte_index = y * row_bytes + x / 8;
            int bit = (data[byte_index] >> (7 - (x % 8))) & 1;
            printf("%d ", bit);
        }
        printf("\n");
    }
    free(data);
}

/*
void convert_pbm_to_1ff(FILE *in) {
    char line[1024];
    // Read PBM header
    do {
        if (!fgets(line, sizeof(line), in)) {
            fprintf(stderr, "Unexpected EOF\n");
            exit(1);
        }
    } while (line[0] == '#');

    int w = 0, h = 0;
    if (sscanf(line, "%d %d", &w, &h) != 2) {
        fprintf(stderr, "Failed to read width/height\n");
        exit(1);
    }

    int row_bytes = (w + 7) / 8;
    uint8_t *data = calloc(h * row_bytes, 1);
    if (!data) {
        perror("calloc");
        exit(1);
    }

    int pixel;
    int x = 0, y = 0;
    while (fscanf(in, "%d", &pixel) == 1) {
        if (pixel != 0 && pixel != 1) continue;

        if (pixel == 1) {
            int byte_index = y * row_bytes + x / 8;
            int bit_offset = 7 - (x % 8);
            data[byte_index] |= (1 << bit_offset);
        }

        if (++x >= w) {
            x = 0;
            y++;
            if (y >= h) break;
        }
    }

    // Output 1ff
    fwrite(MAGIC, 1, 8, stdout);
    write_be32(stdout, w);
    write_be32(stdout, h);
    fwrite(data, 1, h * row_bytes, stdout);
    free(data);
    }*/

void convert_pbm_to_1ff(FILE *in) {
    char line[1024];

    // Read and verify magic
    if (!fgets(line, sizeof(line), in) || strncmp(line, "P1", 2) != 0) {
        fprintf(stderr, "Not a valid P1 PBM file\n");
        exit(1);
    }

    // Skip comment lines and parse width/height
    int w = 0, h = 0;
    while (fgets(line, sizeof(line), in)) {
        if (line[0] == '#') continue;

        if (sscanf(line, "%d %d", &w, &h) == 2) break;
    }

    if (w <= 0 || h <= 0) {
        fprintf(stderr, "Failed to read width/height\n");
        exit(1);
    }

    int row_bytes = (w + 7) / 8;
    uint8_t *data = calloc(h * row_bytes, 1);
    if (!data) {
        perror("calloc");
        exit(1);
    }

    int pixel, x = 0, y = 0;
    while (fscanf(in, "%d", &pixel) == 1) {
        if (pixel != 0 && pixel != 1) continue;

        if (pixel == 1) {
            int byte_index = y * row_bytes + x / 8;
            int bit = 7 - (x % 8);
            data[byte_index] |= (1 << bit);
        }

        if (++x >= w) {
            x = 0;
            if (++y >= h) break;
        }
    }

    // Write 1ff output
    fwrite(MAGIC, 1, 8, stdout);
    write_be32(stdout, w);
    write_be32(stdout, h);
    fwrite(data, 1, h * row_bytes, stdout);
    free(data);
}


int has_ext(const char *name, const char *ext) {
    const char *dot = strrchr(name, '.');
    return dot && strcmp(dot, ext) == 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.1ff|file.pbm > output\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    if (!in) {
        perror("open");
        return 1;
    }

    if (has_ext(argv[1], ".1ff"))
        convert_1ff_to_pbm(in);
    else if (has_ext(argv[1], ".pbm"))
        convert_pbm_to_1ff(in);
    else {
        fprintf(stderr, "Unknown extension: use .pbm or .1ff\n");
        return 1;
    }

    fclose(in);
    return 0;
}

