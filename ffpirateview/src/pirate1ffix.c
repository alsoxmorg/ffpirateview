/*
READ magic header from farbfeld or 1b farbfelds and fixed is it finds the "old" format for 1f
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define    MAGIC "1fpirate"
#define OLDMAGIC "1bfarbfe"
#define FARBFELD "farbfeld"
#define MAGICLEN 8

void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <file.1f> [--fix]\n", prog);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3)
        usage(argv[0]);

    const char *filename = argv[1];
    int fix = (argc == 3 && strcmp(argv[2], "--fix") == 0);

    FILE *fp = fopen(filename, fix ? "r+b" : "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    char magic[MAGICLEN];
    if (fread(magic, 1, MAGICLEN, fp) != MAGICLEN) {
        fprintf(stderr, "Failed to read magic header\n");
        fclose(fp);
        return 1;
    }

    if (memcmp(magic, MAGIC, MAGICLEN) == 0) {
      printf("[OK] Magic is already %s.\n", MAGIC);
    } else if (memcmp(magic, OLDMAGIC, MAGICLEN) == 0) {
        printf("[WARN] Legacy magic \"1bfarbf\\0\" detected\n");
        if (fix) {
            rewind(fp);
            fwrite(MAGIC, 1, MAGICLEN, fp);
            printf("[FIXED] Magic updated to \"%s\"\n", MAGIC);
        } else {
            printf("Use --fix to update magic in-place.\n");
        }
    } else if (memcmp(magic, FARBFELD, MAGICLEN) == 0) {
        printf("[NOTICE] This is a full farbfeld file, not a 1f format.\n");
    } else {
      if (fix) {
	rewind(fp);
	fwrite(MAGIC, 1, MAGICLEN, fp);
	printf("[FIXED] Magic updated to \"%s\"\n", MAGIC);
      }
      printf("[ERROR] Unknown magic: not 1f or farbfeld, be careful with --fix\n");
    }

    fclose(fp);
    return 0;
}
