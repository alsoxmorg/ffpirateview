# Compiler and flags

CC = gcc
# Enable for SDL2
CFLAGS = -I/usr/include/SDL2 -D_REENTRANT
LDFLAGS = -lSDL2

# SDL3 renamed a lot of stuff! sadists!
#this wont work yet without lots of refactoring or #ifdef's
#CFLAGS = -I/usr/local/include/SDL3/ -D_REENTRANT -DSDL3
#LDFLAGS = -L /usr/local/lib/ -lSDL3

# Source and target
SRC = src/ffpirate.c
1FSRC = src/png21f.c
TARGET = src/ffpirate
1FTARGET = src/1fpirate

# Default target
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)     #ffpirate
	$(CC) src/1fpirate.c -o $(1FTARGET) $(CFLAGS) $(LDFLAGS) #1fview
	$(CC) src/1ffedit.c -o src/1ffedit $(CFLAGS) $(LDFLAGS) #-DWITHFARBFELD
	$(CC) src/png21f.c -o src/png21f -lpng             #png21f
	$(CC) -static src/1ffpbm.c -o src/1ffpbm
	$(CC) -static src/resize.c -o src/ffresize
	$(CC) -static src/1ff2ff.c -o src/1ff2ff
	$(CC) -static src/ff21f.c -o src/ff21f
##$(CC) src/ffpirate_lzip.c -o src/ffpiratelz $(CFLAGS) $(LDFLAGS)
# Clean build artifacts

1ffedit: src/1ffedit.c
	$(CC) src/1ffedit.c -o src/1ffedit $(CFLAGS) $(LDFLAGS)
pirate1ffix: src/pirate1ffix.c
	$(CC) src/pirate1ffix.c -o src/pirate1ffix

clean:
	rm -f $(TARGET)
	rm -f $(1FTARGET)
	rm -f src/png21f
	rm -f src/1ffedit
	rm -f src/1ffpbm
	rm -f src/ffresize
	rm -f src/1ff2ff
	rm -f src/ff21f

pirate1f:
	$(CC) src/1fpirate.c -o $(1FTARGET) $(CFLAGS) $(LDFLAGS) #1fview
ffpirate:
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

install:
	cp src/1fpirate ~/.local/bin/
	cp src/ffpirate ~/.local/bin/
	cp src/1ffedit ~/.local/bin/
	cp scripts/zviewer* ~/.local/bin/
	cp src/ffresize ~/.local/bin/
	cp src/1ff2ff ~/.local/bin/
	cp src/png21f ~/.local/bin/
	cp src/ff21f ~/.local/bin/
