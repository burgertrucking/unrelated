.POSIX:

EXE_NAME = unrelated
DLL_NAME = game
DLL = so

CC = cc

PREFIX = /usr/local
SDL_INCLUDE_PATH = $(PREFIX)/include/SDL
LINK_PATH = $(PREFIX)/lib

FLAGS = -g -O0 --std=c89 -Wall -Wextra -Wno-comment -Wno-unused-function -Wno-unused-parameter
FLAGS_SHARED = -fPIC -shared
LIBS = -lSDL
RPATH = -Wl,-rpath,'$$ORIGIN'

all: debugshared

debugshared: main gamedll
main: src/main.c
	$(CC) -DENABLE_HOT_RELOADING $(FLAGS) -I$(SDL_INCLUDE_PATH) -L$(LINK_PATH) $(LIBS) $(RPATH) src/main.c -o $(EXE_NAME)
gamedll: src/game.c
	$(CC) -DENABLE_HOT_RELOADING -DGAME_STANDALONE $(FLAGS) $(FLAGS_SHARED) -I$(SDL_INCLUDE_PATH) -L$(LINK_PATH) $(LIBS) src/game.c -o game.$(DLL)

debugstatic: src/main.c
	$(CC) $(FLAGS) -I$(SDL_INCLUDE_PATH) -L$(LINK_PATH) $(LIBS) src/main.c -o $(EXE_NAME)

clean:
	rm $(EXE_NAME)* *.$(DLL)
