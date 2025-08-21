.POSIX:

CC = cc

PREFIX = /usr/local
LIB_INCLUDE_PATH = $(PREFIX)/include
LINK_PATH = $(PREFIX)/lib
INCLUDE_PATH = src/include

BUILD_DIR = build
MKDIR = mkdir -p
RM = rm -r

CFLAGS = -Wall --std=c89 -g
CFLAGS_SHARED = -fPIC
LDFLAGS =
LDFLAGS_SHARED = -shared
LDLIBS = -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
RPATH = -Wl,-rpath,'$$ORIGIN'

EXE_NAME = unrelated
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

debugstatic: builddir src/main.c
	$(CC) $(CFLAGS) -I$(LIB_INCLUDE_PATH) -I$(INCLUDE_PATH) $(LDFLAGS) -L$(LINK_PATH) $(LDLIBS) src/main.c -o build/$(EXE_NAME)

builddir:
	$(MKDIR) $(BUILD_DIR)

clean:
	$(RM) $(BUILD_DIR)
