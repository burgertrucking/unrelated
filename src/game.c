#include "game.h"
#include "SDL/SDL.h"
#include <stdio.h> /* TEMP for printfs */
#include "includeall.c"

#ifdef _WIN32
    __declspec(dllexport)
#endif
void InitGame(GameState* state)
{
    state->count = 0;
    state->statusFlags = 0; /* clear all flags */
}

#ifdef _WIN32
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
