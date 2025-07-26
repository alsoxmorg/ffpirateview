gcc main.c -o viewer \
    -I/usr/include/SDL2 \
    -D_REENTRANT \
    -lSDL2
gcc view1f.c -o view1f -lSDL2
