/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "statusflags.c"

/* anonymous enum for constants */
enum
{
    FPS = 30,
};

typedef struct GameState
{
    int count; /* temp */
    SDL_Event event;
    Uint8 statusFlags;
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

/* implementation */
#ifdef GAME_STANDALONE /* should be defined when building game.c as a dll */

#include "SDL.h"
#include <stdio.h> /* TEMP for printfs */
#include "all.c"

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
void InitGame(GameState* state)
{
    state->count = 0;
    state->statusFlags = 0; /* clear all flags */
}

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
void UpdateDrawFrame(GameState* state)
{
    ++state->count;
    printf("counter status: %i\n", state->count);
    printf("tick status: %i\n", SDL_GetTicks());

    while (SDL_PollEvent(&state->event))
    {
        switch (state->event.type)
        {
            case SDL_QUIT:
                SetFlag(&state->statusFlags, STATUS_QUIT);
                break;

            case SDL_KEYDOWN:
                switch (state->event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        SetFlag(&state->statusFlags, STATUS_QUIT);
                        break;

                    case SDLK_F5:
                        SetFlag(&state->statusFlags, STATUS_HOT_RELOAD);
                        break;

                    default:
                        break;
                }
                break;
        }
    }

    SDL_Delay(1000 / FPS);
}
#endif /* GAME_STANDALONE */
