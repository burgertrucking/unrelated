.POSIX:

CC = cc

PREFIX = /usr/local
SDL_INCLUDE_PATH = $(PREFIX)/include/SDL
LINK_PATH = $(PREFIX)/lib

CFLAGS = -Wall --std=c89 -g
CFLAGS_SHARED = -fPIC
LDFLAGS =
LDFLAGS_SHARED = -shared
LDLIBS = -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf
RPATH = -Wl,-rpath,'$$ORIGIN'

EXE_NAME = unrelated
DLL_NAME = game
DLL_SUFFIX = so

all: debugshared

debugshared: main gamedll
main: src/main.c
	$(CC) -DENABLE_HOT_RELOADING $(CFLAGS) -I$(SDL_INCLUDE_PATH) $(LDFLAGS) $(RPATH) -L$(LINK_PATH) $(LDLIBS) src/main.c -o $(EXE_NAME)
gamedll: src/game.c
	$(CC) -DENABLE_HOT_RELOADING -DGAME_STANDALONE $(CFLAGS) $(CFLAGS_SHARED) -I$(SDL_INCLUDE_PATH) $(LDFLAGS) $(LDFLAGS_SHARED) $(RPATH) -L$(LINK_PATH) $(LDLIBS) src/game.c -o game.$(DLL_SUFFIX)

debugstatic: src/main.c
	$(CC) $(CFLAGS) -I$(SDL_INCLUDE_PATH) $(LDFLAGS) -L$(LINK_PATH) $(LDLIBS) src/main.c -o $(EXE_NAME)

clean:
	rm $(EXE_NAME)* *.$(DLL_SUFFIX)
