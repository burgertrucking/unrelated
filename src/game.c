/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "input.c"
#include "player.c"

/* anonymous enum for constants */
enum
{
    RES_WIDTH = 640,
    RES_HEIGHT = 480,
    WORLD_RES_WIDTH = 320,
    WORLD_RES_HEIGHT = 240,
    SCREEN_BPP = 32, /* prefer 32 bit colour depth */
    TICK_RATE = 30,
};

typedef struct GameState
{
    Player player;
    SDL_Surface* screen; /* framebuffer surface */
    SDL_Surface* vScreen240; /* 320 x 240 "world screen" for rendering game world */
    SDL_Surface* vScreen480; /* 640 x 480 "game screen" for rendering the game's full image */
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

	state->screen = SDL_SetVideoMode(RES_WIDTH, RES_HEIGHT, SCREEN_BPP, SDL_SWSURFACE|SDL_RESIZABLE|SDL_ANYFORMAT);
	if (!state->screen) return 1; /* STUB */
	SDL_WM_SetCaption("UNRELATED", NULL);
    /* pixel format for current monitor, used for creating game's virtual screens */
	SDL_PixelFormat pf = *state->screen->format;
	state->vScreen240 = SDL_CreateRGBSurface(SDL_SWSURFACE, WORLD_RES_WIDTH, WORLD_RES_HEIGHT,
	                                         pf.BitsPerPixel, pf.Rmask, pf.Gmask, pf.Bmask, pf.Amask);
	state->vScreen480 = SDL_CreateRGBSurface(SDL_SWSURFACE, RES_WIDTH, RES_HEIGHT,
	                                         pf.BitsPerPixel, pf.Rmask, pf.Gmask, pf.Bmask, pf.Amask);
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

    /* vscreen240 (world) */
    /* draw a green background over the world screen */
    SDL_FillRect(state->vScreen240, NULL, SDL_MapRGB(state->vScreen240->format, 0, 180, 60));
    DrawPlayer(&state->player, state->vScreen240);

    /* vscreen480 (game) */
    /* TODO scale vscreen240 to the bounds of vscreen480 */
    /* draw a blue background over the game screen */
    SDL_FillRect(state->vScreen480, NULL, SDL_MapRGB(state->vScreen480->format, 0, 60, 180));
    BlitSurfaceCoords(state->vScreen240, NULL, state->vScreen480, 320/2, 240/2);

    /* screen (the actual window) */
    /* draw a black background over the framebuffer */
    SDL_FillRect(state->screen, NULL, 0);
    /* TODO scale the game screen to be nearest neighbour scaled to the centre of the window */
    BlitSurfaceCoords(state->vScreen480, NULL, state->screen, 0, 0);

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

            SDL_ResizeEvent re;
            case SDL_VIDEORESIZE:
                re = state->event.resize;
                printf("stubberino for video resize, size %d x %d\n", re.w, re.h);
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
