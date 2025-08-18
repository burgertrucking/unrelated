#include "game.h"
#include "SDL/SDL.h"
#include <stdio.h>

#ifdef _WIN32
    __declspec(dllexport)
#endif
void InitGame(GameState* state)
{
    state->count = 0;
    state->shouldQuit = SDL_FALSE;
    state->doHotReload = SDL_FALSE;
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
                state->shouldQuit = SDL_TRUE;
                break;

            case SDL_KEYDOWN:
                switch (state->event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        state->shouldQuit = SDL_TRUE;
                        break;

                    case SDLK_F5:
                        state->doHotReload = SDL_TRUE;
                        break;

                    default:
                        break;
                }
                break;
        }
    }

    SDL_Delay(1000 / FPS);
}
