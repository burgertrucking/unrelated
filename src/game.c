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
    SCREEN_BPP = 0, /* native colour depth */
    TICKS_PER_SECOND = 30,
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

/* anonymous enum for constants (private to module) */
enum
{
    TICK_RATE = 1000 / TICKS_PER_SECOND,
};

static int UpdateGame(GameState* state);
static int DrawGame(GameState* state);
static void HandleEvents(GameState* state);
/* calls SDL_SetVideoMode() with a given width and height; used to get a new screen when resizing the window*/
static SDL_Surface* SetVideoRes(int width, int height);

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int InitGame(GameState* state)
{
    int err = 0;

	state->screen = SetVideoRes(RES_WIDTH, RES_HEIGHT);
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
    Uint32 start = SDL_GetTicks();
    int err = 0;

    /* Update */
    err = UpdateGame(state);

    /* Draw */
    err = DrawGame(state);

    Uint32 end = SDL_GetTicks();
    Uint32 renderTime = end - start;
    Uint32 delay = (renderTime < TICK_RATE)? TICK_RATE - renderTime : 0;

    /* NOTE these make the game run a lot slower, consider adding ingame fps or frametime overlay */
    /* printf("UpdateDrawFrame: time taken to update + render frame: %d ms\n", renderTime); */
    if (!delay) printf("UpdateDrawFrame: running slower than %dfps, time taken is %d ms\n", TICKS_PER_SECOND, renderTime);
    SDL_Delay(delay);

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
    err = DrawPlayer(&state->player, state->vScreen240);

    /* vscreen480 (game) */
    /* scale vscreen240 to size of vscreen480 */
    err = BlitSurfaceScaled(state->vScreen240, NULL, state->vScreen480, 0, 0, 2.0f, 2.0f);

    /* screen (the actual window) */
    /* draw a black background over the framebuffer */
    err = SDL_FillRect(state->screen, NULL, 0);
    /* TODO add option for scaling the game screen to be nearest neighbour scaled to the centre of the window */
    /* currently letterboxes */
    int x, y;
    float w, h;
    if (state->screen->w > state->screen->h)
    {
        h = (float)state->screen->h / RES_HEIGHT;
        w = h;
    }
    else
    {
        /* FIXME crashes */
        w = (float)state->screen->w / RES_WIDTH;
        h = w;
    }
    x = 0, y = 0; /* TODO move to middle of screen */
    err = BlitSurfaceScaled(state->vScreen480, NULL, state->screen, x, y, w, h);

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
                SDL_FreeSurface(state->screen);
                state->screen = SetVideoRes(re.w, re.h);
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

static SDL_Surface* SetVideoRes(int width, int height)
{
    printf("SetVideoRes: Changing screen resolution to %d x %d\n", width, height);
    return SDL_SetVideoMode(width, height, SCREEN_BPP, SDL_SWSURFACE|SDL_RESIZABLE|SDL_ANYFORMAT);
}

#endif /* GAME_STANDALONE */
