include config.mk

.SUFFIXES: .o .c

SRC = 1fpirate_gimp.c \
	file-1fpirate-save.c
#farbfeld.c
OBJ = ${SRC:.c=.o}

all: pirate1f_gimp

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

.c.o:
	${CC} ${INCLUDES} ${CFLAGS} -o $@ -c $<

file-1fpirate-save: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS} ${LDFLAGS}

pirate1f_gimp: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS} ${LDFLAGS}

clean:
	rm -f pirate1f_gimp ${OBJ} *~

install: pirate1f_gimp
	gimptool-2.0 --install-bin $<

uninstall:
	gimptool-2.0 --uninstall-bin pirate1f_gimp

.PHONY: all clean install
