.POSIX:
.SUFFIXES:

CC = cc
PREFIX = /usr/local
LIB_INCLUDE_PATH = $(PREFIX)/include
LINK_PATH = $(PREFIX)/lib
INCLUDE_PATH = src/include
CFLAGS = -Wall --std=c89 -g
LDFLAGS =
LDLIBS = -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf

all: build/obj/main.o build/obj/game.o build/bin
	$(CC) $(LDFLAGS) -L$(LINK_PATH) $(LDLIBS) -o build/bin/a.out build/obj/main.o build/obj/game.o
build/obj/main.o: src/main.c build/obj
	$(CC) $(CFLAGS) -I$(LIB_INCLUDE_PATH) -I$(INCLUDE_PATH) -o build/obj/main.o -c src/main.c
build/obj/game.o: src/game.c build/obj
	$(CC) $(CFLAGS) -I$(LIB_INCLUDE_PATH) -I$(INCLUDE_PATH) -o build/obj/game.o -c src/game.c

build/obj: build
	mkdir build/obj
build/bin: build
	mkdir build/bin
build:
	mkdir build

clean:
	rm -r build
