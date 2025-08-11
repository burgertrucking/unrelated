.POSIX:
.SUFFIXES:

CC = cc
PREFIX = /usr/local
INCLUDE_PATH = $(PREFIX)/include
LINK_PATH = $(PREFIX)/lib
CFLAGS = -Wall --std=c89 -g
LDFLAGS =
LDLIBS = -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf

all: build/obj/main.o build/bin
	$(CC) $(LDFLAGS) -L$(LINK_PATH) $(LDLIBS) -o build/bin/a.out build/obj/main.o
build/obj/main.o: src/main.c build/obj
	$(CC) $(CFLAGS) -I$(INCLUDE_PATH) -o build/obj/main.o -c src/main.c

build/obj: build
	mkdir build/obj
build/bin: build
	mkdir build/bin
build:
	mkdir build

clean:
	rm -r build
