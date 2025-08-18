.POSIX:

CC = cc
PREFIX = /usr/local
LIB_INCLUDE_PATH = $(PREFIX)/include
LINK_PATH = $(PREFIX)/lib
INCLUDE_PATH = src/include
CFLAGS = -Wall --std=c89 -g
CFLAGS_SHARED = -fPIC
LDFLAGS =
LDFLAGS_SHARED = -shared
RPATH = -Wl,-rpath,'$$ORIGIN'
LDLIBS = -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
EXE_NAME = game
DLL_SUFFIX = so

all: debugshared

debugshared: build/obj/main_shared.o build/bin/game.so
	$(CC) $(LDFLAGS) $(RPATH) -L$(LINK_PATH) $(LDLIBS) -o build/bin/$(EXE_NAME) build/obj/main.o
build/bin/game.so: build/obj/game_shared.o
	$(CC) $(LDFLAGS) $(LDFLAGS_SHARED) -L$(LINK_PATH) $(LDLIBS) -o build/bin/game.$(DLL_SUFFIX) build/obj/game.o
build/obj/main_shared.o: src/main.c build/obj
	$(CC) $(CFLAGS) -DENABLE_HOT_RELOADING -I$(LIB_INCLUDE_PATH) -I$(INCLUDE_PATH) -o build/obj/main.o -c src/main.c
build/obj/game_shared.o: src/game.c build/bin
	$(CC) $(CFLAGS_SHARED) -DENABLE_HOT_RELOADING $(CFLAGS) -I$(LIB_INCLUDE_PATH) -I$(INCLUDE_PATH) -o build/obj/game.o -c src/game.c

debugstatic: build/obj/main.o build/obj/game.o build/bin
	$(CC) $(LDFLAGS) -L$(LINK_PATH) $(LDLIBS) -o build/bin/$(EXE_NAME) build/obj/main.o build/obj/game.o
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
