#ifndef GAME_H
#define GAME_H

#include "SDL/SDL.h"

/* anonymous enum for constants */
enum
{
    FPS = 30,
};

typedef struct GameState
{
    int count; /* temp */
    SDL_Event event;
    SDL_bool shouldQuit;
} GameState;

#ifdef _WIN32
    __declspec(dllexport)
#endif
void InitGame(GameState* state);

#ifdef _WIN32
    __declspec(dllexport)
#endif
void UpdateDrawFrame(GameState* state);

#endif /* GAME_H */
