/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "input.c"
#include "player.c"
#include "textdraw.c"
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

typedef struct UserConfig
{
    KeyBinds keys;
} UserConfig;

typedef struct GameState
{
    UserConfig cfg;
    Fonts fonts;
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

#include <stdio.h>
#include "all.c"

/* anonymous enum for constants (private to module) */
enum
{
    TICK_RATE = 1000 / TICKS_PER_SECOND, /* NOTE rounding truncated because SDL_Delay() only takes integers */
    QUIT_TIMER_DURATION = 1*TICKS_PER_SECOND,
};

typedef struct RenderInfo
{
    Uint32 start;
    Uint32 end;
    Uint32 renderTime;
    Uint32 delay;
    float fps;
} RenderInfo;

static RenderInfo rinfo;
static int quitTimer;

static int updateGame(GameState* state);
static int drawGame(GameState* state);
static void handleEvents(GameState* state);
/* calls SDL_SetVideoMode() with a given width and height, and sets relevant state info accordingly */
static int setVideoRes(GameState* state, int width, int height, Uint32 flags);
static int drawVScreenScaled(GameState* state);

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int InitGame(GameState* state)
{
    /* TODO load player config */
    state->cfg.keys = GetDefaultKeyBinds();
    quitTimer = 0;

    int err = setVideoRes(state, RES_WIDTH, RES_HEIGHT, WINDOW_RESIZABLE);
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

    err = InitFonts(&state->fonts);
    err = InitPlayer(&state->player);

    return err;
}

#if defined(_WIN32) && defined(ENABLE_HOT_RELOADING)
    __declspec(dllexport)
#endif
int UpdateDrawFrame(GameState* state)
{
    rinfo.start = SDL_GetTicks();
    int err = 0;

    /* Update */
    err = updateGame(state);

    /* Draw */
    err = drawGame(state);

    /* Clear non-persistent flags */
    ClearFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);

    /* Delay between frames (effective 30 FPS lock) */
    rinfo.end = SDL_GetTicks();
    rinfo.renderTime = rinfo.end - rinfo.start;
    rinfo.delay = (rinfo.renderTime < TICK_RATE)? TICK_RATE - rinfo.renderTime : 0;
    rinfo.fps = 1000.0f / (rinfo.renderTime + rinfo.delay);
    SDL_Delay(rinfo.delay);

    return err;
}

static int updateGame(GameState* state)
{
    /* TODO set up errors for update functions and propagate them */
    int err = 0;

    handleEvents(state);

    UpdatePlayer(&state->player, state->vPad);

    if (CheckFlag(state->statusFlags, STATUS_QUIT_KEY_HELD))
    {
        if (quitTimer >= QUIT_TIMER_DURATION) SetFlag(&state->statusFlags, STATUS_QUIT);
        else ++quitTimer;
    }

    return err;
}

static int drawGame(GameState* state)
{
    int err = 0;

    /* vscreen240 (world) */
    /* draw a green background over the world screen */
    SDL_FillRect(state->vScreen240, NULL, SDL_MapRGB(state->vScreen240->format, 0, 180, 60));
    err = DrawPlayer(&state->player, state->vScreen240);

    /* vscreen480 (game) */
    /* scale vscreen240 to size of vscreen480 */
    err = BlitSurfaceScaled(state->vScreen240, NULL, state->vScreen480, 0, 0, 2.0f, 2.0f);
    /* TEMP checking if text can be drawn */
    const int textStartX = 29*2, textStartY = 170*2; /* start position for textboxes */
    int font = (state->player.isDarkWorld)? FONT_MAIN_DW : FONT_MAIN_LW;
    const char msg[] = "* This town, not that restaurant.\n  It looks weird. I'm not going\n  in...";
    err = DrawText(msg, &state->fonts, state->vScreen480, font, textStartX, textStartY);
    /* TEMP draw fps */
    char frameTimeStr[64];
    sprintf(frameTimeStr, "FPS: %.3f\nRender time: %u ms\n(target < %u)", rinfo.fps, rinfo.renderTime, TICK_RATE);
    if (CheckFlag(state->statusFlags, STATUS_DRAW_FPS))
        err = DrawText(frameTimeStr, &state->fonts, state->vScreen480, FONT_MAIN_DW, 0, 0);
    /* TEMP draw quitting if escape is held */
    /* TODO add and use the quitting font, or just hardcode it as an image to draw */
    if (CheckFlag(state->statusFlags, STATUS_QUIT_KEY_HELD))
        DrawText("QUITTING...", &state->fonts, state->vScreen480, FONT_MAIN_DW, 0, 0);

    /* screen (the actual window) */
    /* draw a black background over the framebuffer */
    err = SDL_FillRect(state->screen, NULL, 0);
    err = drawVScreenScaled(state);

    SDL_Flip(state->screen);
    return err;
}

static void handleEvents(GameState* state)
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
                /* ignore resize events to the current res (prevents fullscreening from undoing itself) */
                if (state->screen->w != re.w && state->screen->h != re.h)
                    setVideoRes(state, re.w, re.h, WINDOW_RESIZABLE);
            break;

            case SDL_KEYDOWN:
                switch (state->event.key.keysym.sym)
                {
                    /* windowing inputs */
                    case SDLK_ESCAPE:
                        SetFlag(&state->statusFlags, STATUS_QUIT_KEY_HELD);
                    break;

                    SDL_Rect** modes;
                    case SDLK_F4:
                        if (CheckFlag(state->statusFlags, STATUS_FULLSCREEN))
                        {
                            ClearFlag(&state->statusFlags, STATUS_FULLSCREEN);
                            /* TODO save previous non-fullscreen scale instead of hardcoding to 2x */
                            setVideoRes(state, RES_WIDTH, RES_HEIGHT, WINDOW_RESIZABLE);
                        }
                        else
                        {
                            SetFlag(&state->statusFlags, STATUS_FULLSCREEN);
                            modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
                            /* TEMP hardcoded to highest res */
                            setVideoRes(state, modes[0]->w, modes[0]->h, WINDOW_FULLSCREEN);
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
                    case SDLK_f: /* NOTE conflicts with esdf movement if g is pressed */
                        ToggleFlag(&state->statusFlags, STATUS_DRAW_FPS);
                    break;
                    case SDLK_g:
                        state->player.isDarkWorld = !state->player.isDarkWorld;
                    break;
                    case SDLK_h:
                        /* TEMP rebinding esdf to wasd to test key rebinding */
                        printf("handleEvents: changing WASD to ESDF\n");
                        state->cfg.keys.upA = SDLK_e;
                        state->cfg.keys.leftA = SDLK_s;
                        state->cfg.keys.downA = SDLK_d;
                        state->cfg.keys.rightA = SDLK_f;
                    break;
                    /* TODO make these scaling options in the settings rather than hardcoded keypresses */
                    case SDLK_1:
                        setVideoRes(state, WORLD_RES_WIDTH, WORLD_RES_HEIGHT, WINDOW_RESIZABLE);
                    break;

                    case SDLK_2:
                        setVideoRes(state, WORLD_RES_WIDTH*2, WORLD_RES_HEIGHT*2, WINDOW_RESIZABLE);
                    break;

                    case SDLK_3:
                        setVideoRes(state, WORLD_RES_WIDTH*3, WORLD_RES_HEIGHT*3, WINDOW_RESIZABLE);
                    break;

                    case SDLK_4:
                        setVideoRes(state, WORLD_RES_WIDTH*4, WORLD_RES_HEIGHT*4, WINDOW_RESIZABLE);
                    break;

                    case SDLK_5:
                        setVideoRes(state, WORLD_RES_WIDTH*5, WORLD_RES_HEIGHT*5, WINDOW_RESIZABLE);
                    break;

                    case SDLK_6:
                        setVideoRes(state, WORLD_RES_WIDTH*6, WORLD_RES_HEIGHT*6, WINDOW_RESIZABLE);
                    break;

                    case SDLK_7:
                        setVideoRes(state, WORLD_RES_WIDTH*7, WORLD_RES_HEIGHT*7, WINDOW_RESIZABLE);
                    break;

                    case SDLK_8:
                        setVideoRes(state, WORLD_RES_WIDTH*8, WORLD_RES_HEIGHT*8, WINDOW_RESIZABLE);
                    break;

                    case SDLK_9:
                        setVideoRes(state, WORLD_RES_WIDTH*9, WORLD_RES_HEIGHT*9, WINDOW_RESIZABLE);
                    break;

                    /* TODO add UT debug mode inputs */

                    default:
                    break;
                }
            break;

            case SDL_KEYUP:
                switch (state->event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        ClearFlag(&state->statusFlags, STATUS_QUIT_KEY_HELD);
                        quitTimer = 0;
                    break;

                    default:
                    break;
                }
            break;
        }

        /* handle player input (done here since switch statements must use compile time constants) */
        SDLKey key = state->event.key.keysym.sym;
        if (state->event.type == SDL_KEYDOWN)
        {
            if (key == state->cfg.keys.down) SetFlag(&state->vPad, VKEY_DOWN);
            if (key == state->cfg.keys.up) SetFlag(&state->vPad, VKEY_UP);
            if (key == state->cfg.keys.left) SetFlag(&state->vPad, VKEY_LEFT);
            if (key == state->cfg.keys.right) SetFlag(&state->vPad, VKEY_RIGHT);
            if (key == state->cfg.keys.accept) SetFlag(&state->vPad, VKEY_ACCEPT);
            if (key == state->cfg.keys.cancel) SetFlag(&state->vPad, VKEY_CANCEL);
            if (key == state->cfg.keys.menu) SetFlag(&state->vPad, VKEY_MENU);

            if (key == state->cfg.keys.downA) SetFlag(&state->vPad, VKEY_DOWN_A);
            if (key == state->cfg.keys.upA) SetFlag(&state->vPad, VKEY_UP_A);
            if (key == state->cfg.keys.leftA) SetFlag(&state->vPad, VKEY_LEFT_A);
            if (key == state->cfg.keys.rightA) SetFlag(&state->vPad, VKEY_RIGHT_A);
            if (key == state->cfg.keys.acceptA) SetFlag(&state->vPad, VKEY_ACCEPT_A);
            if (key == state->cfg.keys.cancelA) SetFlag(&state->vPad, VKEY_CANCEL_A);
            if (key == state->cfg.keys.menuA) SetFlag(&state->vPad, VKEY_MENU_A);

            if (key == state->cfg.keys.downB) SetFlag(&state->vPad, VKEY_DOWN_B);
            if (key == state->cfg.keys.upB) SetFlag(&state->vPad, VKEY_UP_B);
            if (key == state->cfg.keys.leftB) SetFlag(&state->vPad, VKEY_LEFT_B);
            if (key == state->cfg.keys.rightB) SetFlag(&state->vPad, VKEY_RIGHT_B);
            if (key == state->cfg.keys.acceptB) SetFlag(&state->vPad, VKEY_ACCEPT_B);
            if (key == state->cfg.keys.cancelB) SetFlag(&state->vPad, VKEY_CANCEL_B);
            if (key == state->cfg.keys.menuB) SetFlag(&state->vPad, VKEY_MENU_B);
        }
        else if (state->event.type == SDL_KEYUP)
        {
            if (key == state->cfg.keys.down) ClearFlag(&state->vPad, VKEY_DOWN);
            if (key == state->cfg.keys.up) ClearFlag(&state->vPad, VKEY_UP);
            if (key == state->cfg.keys.left) ClearFlag(&state->vPad, VKEY_LEFT);
            if (key == state->cfg.keys.right) ClearFlag(&state->vPad, VKEY_RIGHT);
            if (key == state->cfg.keys.accept) ClearFlag(&state->vPad, VKEY_ACCEPT);
            if (key == state->cfg.keys.cancel) ClearFlag(&state->vPad, VKEY_CANCEL);
            if (key == state->cfg.keys.menu) ClearFlag(&state->vPad, VKEY_MENU);

            if (key == state->cfg.keys.downA) ClearFlag(&state->vPad, VKEY_DOWN_A);
            if (key == state->cfg.keys.upA) ClearFlag(&state->vPad, VKEY_UP_A);
            if (key == state->cfg.keys.leftA) ClearFlag(&state->vPad, VKEY_LEFT_A);
            if (key == state->cfg.keys.rightA) ClearFlag(&state->vPad, VKEY_RIGHT_A);
            if (key == state->cfg.keys.acceptA) ClearFlag(&state->vPad, VKEY_ACCEPT_A);
            if (key == state->cfg.keys.cancelA) ClearFlag(&state->vPad, VKEY_CANCEL_A);
            if (key == state->cfg.keys.menuA) ClearFlag(&state->vPad, VKEY_MENU_A);

            if (key == state->cfg.keys.downB) ClearFlag(&state->vPad, VKEY_DOWN_B);
            if (key == state->cfg.keys.upB) ClearFlag(&state->vPad, VKEY_UP_B);
            if (key == state->cfg.keys.leftB) ClearFlag(&state->vPad, VKEY_LEFT_B);
            if (key == state->cfg.keys.rightB) ClearFlag(&state->vPad, VKEY_RIGHT_B);
            if (key == state->cfg.keys.acceptB) ClearFlag(&state->vPad, VKEY_ACCEPT_B);
            if (key == state->cfg.keys.cancelB) ClearFlag(&state->vPad, VKEY_CANCEL_B);
            if (key == state->cfg.keys.menuB) ClearFlag(&state->vPad, VKEY_MENU_B);
        }
    }
}

static int setVideoRes(GameState* state, int width, int height, Uint32 flags)
{
    if (!flags)
    {
        fprintf(stderr, "WARNING: setVideoRes: No flags specified, using DEFAULT_VIDEO_FLAGS as fallback\n");
        flags = DEFAULT_VIDEO_FLAGS;
    }
    printf("setVideoRes: Changing screen resolution to %d x %d\n", width, height);
    state->screen = SDL_SetVideoMode(width, height, SCREEN_BPP, flags);
    if (!state->screen) return 1;
    SDL_WM_GrabInput(SDL_GRAB_OFF); /* prevent sdl12-compat default behaviour of capturing mouse input */
    SDL_ShowCursor(0);
    SetFlag(&state->statusFlags, STATUS_WINDOW_RESIZED);
    return 0;
}

static int drawVScreenScaled(GameState* state)
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
