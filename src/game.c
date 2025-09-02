/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "input.c"
#include "player.c"

/* anonymous enum for constants */
enum
{
    RES_WIDTH = 320,
    RES_HEIGHT = 240,
    SCREEN_BPP = 0, /* native bpp */
    TICK_RATE = 30,
};

typedef struct GameState
{
    Player player;
    SDL_Surface* screen;
    SDL_Event event;
    Uint8 vPad;
    Uint8 statusFlags;
} GameState;

#ifdef _WIN32
    __declspec(dllexport)
#endif
int InitGame(GameState* state);

#ifdef _WIN32
    __declspec(dllexport)
#endif
int UpdateDrawFrame(GameState* state);

#endif /* GAME_H */

/* implementation */
#ifdef GAME_STANDALONE /* should be defined when building game.c as a dll */

#include "SDL.h"
#include "all.c"

static int UpdateGame(GameState* state);
static int DrawGame(GameState* state);
static void HandleEvents(GameState* state);

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int InitGame(GameState* state)
{
    int err = 0;

	state->screen = SDL_SetVideoMode(RES_WIDTH, RES_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if (!state->screen) return 1; /* STUB */
	SDL_WM_SetCaption("UNRELATED", NULL);

    state->statusFlags = 0; /* clear all flags */

    err = InitPlayer(&state->player);

    return err;
}

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int UpdateDrawFrame(GameState* state)
{
    int err = 0;

    /* Update */
    err = UpdateGame(state);

    /* Draw */
    err = DrawGame(state);

    SDL_Delay(1000 / TICK_RATE);

    return err;
}

static int UpdateGame(GameState* state)
{
    int err = 0;

    HandleEvents(state);

    UpdatePlayer(&state->player, state->vPad);

    return err;
}

static int DrawGame(GameState* state)
{
    int err = 0;

    /* draw a green background over the screen */
    SDL_FillRect(state->screen, &state->screen->clip_rect, SDL_MapRGB(state->screen->format, 0, 180, 60));
    DrawPlayer(&state->player, state->screen);

    SDL_Flip(state->screen);
    return err;
}

static void HandleEvents(GameState* state)
{
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

                    case SDLK_DOWN:
                        SetFlag(&state->vPad, VKEY_DOWN);
                    break;

                    case SDLK_UP:
                        SetFlag(&state->vPad, VKEY_UP);
                    break;

                    case SDLK_LEFT:
                        SetFlag(&state->vPad, VKEY_LEFT);
                    break;

                    case SDLK_RIGHT:
                        SetFlag(&state->vPad, VKEY_RIGHT);
                    break;

                    case SDLK_x:
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT:
                        /* NOTE consider making key aliases so eg pressing shift while x is held still counts */
                        SetFlag(&state->vPad, VKEY_CANCEL);
                    break;

                    default:
                    break;
                }
            break;

            case SDL_KEYUP:
                switch (state->event.key.keysym.sym)
                {
                    case SDLK_DOWN:
                        ClearFlag(&state->vPad, VKEY_DOWN);
                    break;

                    case SDLK_UP:
                        ClearFlag(&state->vPad, VKEY_UP);
                    break;

                    case SDLK_LEFT:
                        ClearFlag(&state->vPad, VKEY_LEFT);
                    break;

                    case SDLK_RIGHT:
                        ClearFlag(&state->vPad, VKEY_RIGHT);
                    break;

                    case SDLK_x:
                    case SDLK_LSHIFT:
                    case SDLK_RSHIFT:
                        ClearFlag(&state->vPad, VKEY_CANCEL);
                    break;

                    default:
                    break;
                }
            break;
        }
    }
}

#endif /* GAME_STANDALONE */
