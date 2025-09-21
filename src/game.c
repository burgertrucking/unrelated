/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "input.c"
#include "player.c"
#include "statusflag.h"

/* anonymous enum for constants */
enum
{
    RES_WIDTH = 640,
    RES_HEIGHT = 480,
    WORLD_RES_WIDTH = 320,
    WORLD_RES_HEIGHT = 240,
    SCREEN_BPP = 0, /* native colour depth */
    TICKS_PER_SECOND = 30,
    DEFAULT_VIDEO_FLAGS = SDL_SWSURFACE|SDL_ANYFORMAT, /* NOTE may eventually let machine decide hw vs sw rather than hardcoding */
    WINDOW_RESIZABLE = SDL_RESIZABLE|DEFAULT_VIDEO_FLAGS,
    WINDOW_FULLSCREEN = SDL_FULLSCREEN|DEFAULT_VIDEO_FLAGS,
};

typedef struct GameState
{
    Player player;
    SDL_Surface* screen; /* framebuffer surface */
    SDL_Surface* vScreen240; /* 320 x 240 "world screen" for rendering game world */
    SDL_Surface* vScreen480; /* 640 x 480 "game screen" for rendering the game's full image */
    SDL_Event event;
    Uint32 vPad;
    Uint32 statusFlags;
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
/* calls SDL_SetVideoMode() with a given width and height, and sets relevant state info accordingly */
static int SetVideoRes(GameState* state, int width, int height, Uint32 flags);
static int DrawVScreenScaled(GameState* state);

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int InitGame(GameState* state)
{
    int err = SetVideoRes(state, RES_WIDTH, RES_HEIGHT, WINDOW_RESIZABLE);
	if (err) return err;
	SDL_WM_SetCaption("UNRELATED", NULL);
    /* pixel format for current monitor, used for creating game's virtual screens */
	SDL_PixelFormat pf = *state->screen->format;
	state->vScreen240 = SDL_CreateRGBSurface(SDL_SWSURFACE, WORLD_RES_WIDTH, WORLD_RES_HEIGHT,
	                                         pf.BitsPerPixel, pf.Rmask, pf.Gmask, pf.Bmask, pf.Amask);
	state->vScreen480 = SDL_CreateRGBSurface(SDL_SWSURFACE, RES_WIDTH, RES_HEIGHT,
	                                         pf.BitsPerPixel, pf.Rmask, pf.Gmask, pf.Bmask, pf.Amask);
    state->statusFlags = 0; /* clear all flags */
    state->vPad = 0; /* clear all virtual pad inputs, prevents ghost inputs at startup */

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

    /* Clear non-persistent flags */
    ClearFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);

    /* Delay between frames (effective 30 FPS lock) */
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
    /* TODO set up errors for update functions and propagate them */
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
    err = DrawVScreenScaled(state);

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
                SetVideoRes(state, re.w, re.h, WINDOW_RESIZABLE);
            break;

            case SDL_KEYDOWN:
                switch (state->event.key.keysym.sym)
                {
                    /* windowing inputs */
                    case SDLK_ESCAPE:
                        SetFlag(&state->statusFlags, STATUS_QUIT);
                    break;

                    SDL_Rect** modes;
                    case SDLK_F4:
                        if (CheckFlag(state->statusFlags, STATUS_FULLSCREEN))
                        {
                            ClearFlag(&state->statusFlags, STATUS_FULLSCREEN);
                            /* TODO save previous non-fullscreen scale instead of hardcoding to 2x */
                            SetVideoRes(state, RES_WIDTH, RES_HEIGHT, WINDOW_RESIZABLE);
                        }
                        else
                        {
                            SetFlag(&state->statusFlags, STATUS_FULLSCREEN);
                            modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
                            /* TEMP hardcoded to highest res */
                            SetVideoRes(state, modes[0]->w, modes[0]->h, WINDOW_FULLSCREEN);
                        }
                        SetFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);
                    break;

                    /* TODO add alt + enter for fullscreen */


                    /* debug inputs */
                    /* unique to this game */
                    case SDLK_F5:
                        SetFlag(&state->statusFlags, STATUS_HOT_RELOAD);
                    break;

                    /* NOTE these inputs may conflict with ut debug mode inputs */
                    case SDLK_d:
                        state->player.isDarkWorld = !state->player.isDarkWorld;
                        printf("DEBUG STUB switching player form: dw == %i\n", state->player.isDarkWorld);
                    break;
                    /* TODO make these scaling options in the settings rather than hardcoded keypresses */
                    case SDLK_1:
                        SetVideoRes(state, WORLD_RES_WIDTH, WORLD_RES_HEIGHT, WINDOW_RESIZABLE);
                    break;

                    case SDLK_2:
                        SetVideoRes(state, WORLD_RES_WIDTH*2, WORLD_RES_HEIGHT*2, WINDOW_RESIZABLE);
                    break;

                    case SDLK_3:
                        SetVideoRes(state, WORLD_RES_WIDTH*3, WORLD_RES_HEIGHT*3, WINDOW_RESIZABLE);
                    break;

                    case SDLK_4:
                        SetVideoRes(state, WORLD_RES_WIDTH*4, WORLD_RES_HEIGHT*4, WINDOW_RESIZABLE);
                    break;

                    case SDLK_5:
                        SetVideoRes(state, WORLD_RES_WIDTH*5, WORLD_RES_HEIGHT*5, WINDOW_RESIZABLE);
                    break;

                    case SDLK_6:
                        SetVideoRes(state, WORLD_RES_WIDTH*6, WORLD_RES_HEIGHT*6, WINDOW_RESIZABLE);
                    break;

                    case SDLK_7:
                        SetVideoRes(state, WORLD_RES_WIDTH*7, WORLD_RES_HEIGHT*7, WINDOW_RESIZABLE);
                    break;

                    case SDLK_8:
                        SetVideoRes(state, WORLD_RES_WIDTH*8, WORLD_RES_HEIGHT*8, WINDOW_RESIZABLE);
                    break;

                    case SDLK_9:
                        SetVideoRes(state, WORLD_RES_WIDTH*9, WORLD_RES_HEIGHT*9, WINDOW_RESIZABLE);
                    break;

                    /* TODO add UT debug mode inputs */


                    /* control inputs */
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

                    /* FIXME releasing one of these aliased keys will count as a depress even if another alias is
                    still held down */
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

static int SetVideoRes(GameState* state, int width, int height, Uint32 flags)
{
    if (!flags)
    {
        fprintf(stderr, "WARNING: SetVideoRes: No flags specified, using DEFAULT_VIDEO_FLAGS as fallback\n");
        flags = DEFAULT_VIDEO_FLAGS;
    }
    printf("SetVideoRes: Changing screen resolution to %d x %d\n", width, height);
    state->screen = SDL_SetVideoMode(width, height, SCREEN_BPP, flags);
    if (!state->screen) return 1;
    SDL_WM_GrabInput(SDL_GRAB_OFF); /* prevent sdl12-compat default behaviour of capturing mouse input */
    SDL_ShowCursor(0);
    SetFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);
    return 0;
}

static int DrawVScreenScaled(GameState* state)
{
    /* TODO add option for scaling the game screen to be nearest neighbour scaled to the centre of the window */
    /* right now this is worked around by having number keys integer scale that amount (based on world res) */
    /* scaling currently only letterboxes */
    static int x, y;
    static float scale;
    static SDL_bool landscape, resized = SDL_TRUE;
    if (CheckFlag(state->statusFlags, STATUS_WINDOW_RESIZED))
    {
        ClearFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);
        resized = SDL_TRUE;
    }
    if (resized)
    {
         /* if screen aspect ratio is >= 4:3, it's considered landscape */
        landscape = state->screen->w >= state->screen->h*4.0f/3.0f;
        if (landscape)
        {
            /* scale based on height */
            scale = (float)state->screen->h / RES_HEIGHT;
            x = (state->screen->w - RES_WIDTH*scale)/2.0f;
            y = 0;
        }
        else
        {
            /* scale based on width */
            scale = (float)state->screen->w / RES_WIDTH;
            y = (state->screen->h - RES_HEIGHT*scale)/2.0f;
            x = 0;
        }
        resized = SDL_FALSE;
    }
    return BlitSurfaceScaled(state->vScreen480, NULL, state->screen, x, y, scale, scale);
}

#endif /* GAME_STANDALONE */
