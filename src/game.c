/* header */
#ifndef GAME_H
#define GAME_H

#include "SDL.h"
#include "input.c"
#include "player.c"
#include "textdraw.c"
#include "room.c"
#include "textbox.c"
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
    Room room;
    Textbox textbox;
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
	SDL_EnableKeyRepeat(0, 0);
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
    err = InitTextbox(&state->textbox);
    /* TEMP init room walls (InitRoom() doesn't handle this) */
    /* NOTE though these sizes are very close to exact, they are still too fat for the player to fit through */
    state->room.wallsLen = 16;
    state->room.walls[0] = (Rect){ 69, 123, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[1] = (Rect){ 69, 154, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[2] = (Rect){ 69, 188, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[3] = (Rect){ 139, 188, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[4] = (Rect){ 139, 154, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[5] = (Rect){ 139, 123, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[6] = (Rect){ 206, 123, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[7] = (Rect){ 206, 154, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[8] = (Rect){ 206, 188, 1.9f*TILE_SIZE, 0.8f*TILE_SIZE };
    state->room.walls[9] = (Rect){ 101, 89, 4.63f*TILE_SIZE, 0.79f*TILE_SIZE };
    state->room.walls[10] = (Rect){ 18, 23, TILE_SIZE, 10.63f*TILE_SIZE };
    state->room.walls[11] = (Rect){ 38, 218, 12.3f*TILE_SIZE, 0.85f*TILE_SIZE };
    state->room.walls[12] = (Rect){ 286, 23, 0.85f*TILE_SIZE, 10.63f*TILE_SIZE };
    state->room.walls[13] = (Rect){ 22, 3, 13.96f*TILE_SIZE, 0.95f*TILE_SIZE };
    state->room.walls[14] = (Rect){ 269, 52, TILE_SIZE, TILE_SIZE };
    state->room.walls[15] = (Rect){ 38, 52, 9.89f*TILE_SIZE, TILE_SIZE };
    /* TEMP init room interactables */
    /* only some of them used for testing */
    state->room.interactablesLen = 3;
    state->room.interactables[0] = (Rect){ 211, 52, 0.9473684*TILE_SIZE, 0.9*TILE_SIZE };
    state->room.interactables[1] = (Rect){ 103, 89, 0.55*TILE_SIZE, 0.368421*TILE_SIZE };
    state->room.interactables[2] = (Rect){ 167, 92, 0.9*TILE_SIZE, 0.45*TILE_SIZE };
    InitRoom(&state->room, "res/rip/bg/alphysclass.png", ROOM_SHEET_WHOLE);

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
    /* clear current frame's keydown status (leaving only held) */
    if (CheckFlag(state->vPad, VKEY_DOWN)) ClearFlag(&state->vPad, VKEY_DOWN);
    if (CheckFlag(state->vPad, VKEY_RIGHT)) ClearFlag(&state->vPad, VKEY_RIGHT);
    if (CheckFlag(state->vPad, VKEY_UP)) ClearFlag(&state->vPad, VKEY_UP);
    if (CheckFlag(state->vPad, VKEY_LEFT)) ClearFlag(&state->vPad, VKEY_LEFT);
    if (CheckFlag(state->vPad, VKEY_ACCEPT)) ClearFlag(&state->vPad, VKEY_ACCEPT);
    if (CheckFlag(state->vPad, VKEY_CANCEL)) ClearFlag(&state->vPad, VKEY_CANCEL);
    if (CheckFlag(state->vPad, VKEY_MENU)) ClearFlag(&state->vPad, VKEY_MENU);
    if (CheckFlag(state->vPad, VKEY_ACCEPT_A)) ClearFlag(&state->vPad, VKEY_ACCEPT_A);
    if (CheckFlag(state->vPad, VKEY_CANCEL_A)) ClearFlag(&state->vPad, VKEY_CANCEL_A);
    if (CheckFlag(state->vPad, VKEY_MENU_A)) ClearFlag(&state->vPad, VKEY_MENU_A);
    if (CheckFlag(state->vPad, VKEY_ACCEPT_B)) ClearFlag(&state->vPad, VKEY_ACCEPT_B);
    if (CheckFlag(state->vPad, VKEY_CANCEL_B)) ClearFlag(&state->vPad, VKEY_CANCEL_B);
    if (CheckFlag(state->vPad, VKEY_MENU_B)) ClearFlag(&state->vPad, VKEY_MENU_B);


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

    UpdatePlayer(&state->player, &state->room, &state->textbox, state->vPad, &state->statusFlags);
    UpdateTextbox(&state->textbox, state->vPad, &state->statusFlags);

    /* update quit timer */
    if (CheckFlag(state->statusFlags, STATUS_QUIT_KEY_HELD))
    {
        if (quitTimer >= QUIT_TIMER_DURATION) SetFlag(&state->statusFlags, STATUS_QUIT);
        else ++quitTimer;
    }

    return err;
}

static int drawGame(GameState* state)
{
    /* TODO decompose into more granluar functions (eg drawing per screen, drawing gizmos) */
    int err = 0;

    /* vscreen240 (world) */
    /* TODO handle drawing room depending on screen size (some are optimised for 480p) */
    err = DrawRoom(&state->room, state->vScreen240);
    err = DrawPlayer(&state->player, state->vScreen240, state->statusFlags);
    /* draw gizmos for vscreen240 */
    if (CheckFlag(state->statusFlags, STATUS_DRAW_GIZMOS))
    {
        err = DrawRoomGizmos(&state->room, state->vScreen240);
        err = DrawPlayerGizmos(&state->player, state->vScreen240);
    }

    /* vscreen480 (game) */
    /* scale vscreen240 to size of vscreen480 */
    err = BlitSurfaceScaled(state->vScreen240, NULL, state->vScreen480, (Vec2){0}, (Vec2){2.0f, 2.0f});
    /* TEMP checking if text can be drawn */
    /* TEMP draw fps */
    char ftstrData[64];
    sprintf(ftstrData, "FPS: %.3f\nRender time: %u ms\n(target < %u)", rinfo.fps, rinfo.renderTime, TICK_RATE);
    String frameTimeStr = (String) { ftstrData, 64 };
    if (CheckFlag(state->statusFlags, STATUS_DRAW_FPS))
        err = DrawText(frameTimeStr, &state->fonts, state->vScreen480, FONT_MAIN_DW, (Vec2){0});
    err = DrawTextbox(&state->textbox, CheckFlag(state->statusFlags, STATUS_IS_DARK_WORLD), &state->fonts, state->vScreen480);
    /* TEMP draw quitting if escape is held */
    /* TODO add and use the quitting font, or just hardcode it as an image to draw */
    if (CheckFlag(state->statusFlags, STATUS_QUIT_KEY_HELD))
    {
        String quitText = (String){ "QUITTING...", sizeof("QUITTING...") };
        DrawText(quitText, &state->fonts, state->vScreen480, FONT_MAIN_DW, (Vec2){0});
    }

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

                    case SDLK_F6:
                        printf("Reload game requested, re-running InitGame()\n");
                        InitGame(state);
                    break;

                    /* NOTE these inputs may conflict with ut debug mode inputs */
                    case SDLK_f: /* NOTE conflicts with esdf movement if g is pressed */
                        ToggleFlag(&state->statusFlags, STATUS_DRAW_FPS);
                    break;
                    case SDLK_g:
                        ToggleFlag(&state->statusFlags, STATUS_IS_DARK_WORLD);
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

                    /* UT debug mode inputs */
                    case SDLK_v:
                        ToggleFlag(&state->statusFlags, STATUS_DRAW_GIZMOS);
                    break;

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

        /* check for key down or key up events */
        SDLKey key = state->event.key.keysym.sym;
        if (state->event.type == SDL_KEYDOWN)
        {
            if (key == state->cfg.keys.down)
            {
                SetFlag(&state->vPad, VKEY_DOWN);
                SetFlag(&state->vPad, VKEY_DOWN_HELD);
            }
            if (key == state->cfg.keys.up)
            {
                SetFlag(&state->vPad, VKEY_UP);
                SetFlag(&state->vPad, VKEY_UP_HELD);
            }
            if (key == state->cfg.keys.left)
            {
                SetFlag(&state->vPad, VKEY_LEFT);
                SetFlag(&state->vPad, VKEY_LEFT_HELD);
            }
            if (key == state->cfg.keys.right)
            {
                SetFlag(&state->vPad, VKEY_RIGHT);
                SetFlag(&state->vPad, VKEY_RIGHT_HELD);
            }
            if (key == state->cfg.keys.accept)
            {
                SetFlag(&state->vPad, VKEY_ACCEPT);
                SetFlag(&state->vPad, VKEY_ACCEPT_HELD);
            }
            if (key == state->cfg.keys.cancel)
            {
                SetFlag(&state->vPad, VKEY_CANCEL);
                SetFlag(&state->vPad, VKEY_CANCEL_HELD);
            }
            if (key == state->cfg.keys.menu)
            {
                SetFlag(&state->vPad, VKEY_MENU);
                SetFlag(&state->vPad, VKEY_MENU_HELD);
            }

            if (key == state->cfg.keys.acceptA)
            {
                SetFlag(&state->vPad, VKEY_ACCEPT_A);
                SetFlag(&state->vPad, VKEY_ACCEPT_A_HELD);
            }
            if (key == state->cfg.keys.cancelA)
            {
                SetFlag(&state->vPad, VKEY_CANCEL_A);
                SetFlag(&state->vPad, VKEY_CANCEL_A_HELD);
            }
            if (key == state->cfg.keys.menuA)
            {
                SetFlag(&state->vPad, VKEY_MENU_A);
                SetFlag(&state->vPad, VKEY_MENU_A_HELD);
            }

            if (key == state->cfg.keys.acceptB)
            {
                SetFlag(&state->vPad, VKEY_ACCEPT_B);
                SetFlag(&state->vPad, VKEY_ACCEPT_B_HELD);
            }
            if (key == state->cfg.keys.cancelB)
            {
                SetFlag(&state->vPad, VKEY_CANCEL_B);
                SetFlag(&state->vPad, VKEY_CANCEL_B_HELD);
            }
            if (key == state->cfg.keys.menuB)
            {
                SetFlag(&state->vPad, VKEY_MENU_B);
                SetFlag(&state->vPad, VKEY_MENU_B_HELD);
            }
        }
        else if (state->event.type == SDL_KEYUP)
        {
            /* Non-held flags are cleared at the end of every frame */
            if (key == state->cfg.keys.down) ClearFlag(&state->vPad, VKEY_DOWN_HELD);
            if (key == state->cfg.keys.up) ClearFlag(&state->vPad, VKEY_UP_HELD);
            if (key == state->cfg.keys.left) ClearFlag(&state->vPad, VKEY_LEFT_HELD);
            if (key == state->cfg.keys.right) ClearFlag(&state->vPad, VKEY_RIGHT_HELD);
            if (key == state->cfg.keys.accept) ClearFlag(&state->vPad, VKEY_ACCEPT_HELD);
            if (key == state->cfg.keys.cancel) ClearFlag(&state->vPad, VKEY_CANCEL_HELD);
            if (key == state->cfg.keys.menu) ClearFlag(&state->vPad, VKEY_MENU_HELD);

            if (key == state->cfg.keys.acceptA) ClearFlag(&state->vPad, VKEY_ACCEPT_A_HELD);
            if (key == state->cfg.keys.cancelA) ClearFlag(&state->vPad, VKEY_CANCEL_A_HELD);
            if (key == state->cfg.keys.menuA) ClearFlag(&state->vPad, VKEY_MENU_A_HELD);

            if (key == state->cfg.keys.acceptB) ClearFlag(&state->vPad, VKEY_ACCEPT_B_HELD);
            if (key == state->cfg.keys.cancelB) ClearFlag(&state->vPad, VKEY_CANCEL_B_HELD);
            if (key == state->cfg.keys.menuB) ClearFlag(&state->vPad, VKEY_MENU_B_HELD);
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
    static Vec2 pos;
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
            pos.x = (state->screen->w - RES_WIDTH*scale)/2.0f;
            pos.y = 0;
        }
        else
        {
            /* scale based on width */
            scale = (float)state->screen->w / RES_WIDTH;
            pos.y = (state->screen->h - RES_HEIGHT*scale)/2.0f;
            pos.x = 0;
        }
        resized = SDL_FALSE;
    }
    return BlitSurfaceScaled(state->vScreen480, NULL, state->screen, pos, (Vec2){scale, scale});
}

#endif /* GAME_STANDALONE */
