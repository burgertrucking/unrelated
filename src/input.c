/* header */
#ifndef INPUT_H
#define INPUT_H

#include "SDL.h"

typedef enum InputButton
{
    VKEY_DOWN = 1 << 0,
    VKEY_RIGHT = 1 << 1,
    VKEY_UP = 1 << 2,
    VKEY_LEFT = 1 << 3,
    VKEY_ACCEPT = 1 << 4,
    VKEY_CANCEL = 1 << 5,
    VKEY_MENU = 1 << 6,

    /* Each button gets two extra aliases (eg. Enter, Shift, Control) like in the original */
    VKEY_ACCEPT_A = 1 << 7,
    VKEY_CANCEL_A = 1 << 8,
    VKEY_MENU_A = 1 << 9,

    VKEY_ACCEPT_B = 1 << 10,
    VKEY_CANCEL_B = 1 << 11,
    VKEY_MENU_B = 1 << 12,

    /* These get set to 1 if a button has been held down for multiple consecutive frames */
    VKEY_DOWN_HELD = 1 << 13,
    VKEY_RIGHT_HELD = 1 << 14,
    VKEY_UP_HELD = 1 << 15,
    VKEY_LEFT_HELD = 1 << 16,
    VKEY_ACCEPT_HELD = 1 << 17,
    VKEY_CANCEL_HELD = 1 << 18,
    VKEY_MENU_HELD = 1 << 19,

    VKEY_ACCEPT_A_HELD = 1 << 20,
    VKEY_CANCEL_A_HELD = 1 << 21,
    VKEY_MENU_A_HELD = 1 << 22,

    VKEY_ACCEPT_B_HELD = 1 << 23,
    VKEY_CANCEL_B_HELD = 1 << 24,
    VKEY_MENU_B_HELD = 1 << 25,

} InputButton;

typedef struct KeyBinds
{
    Uint16 up;
    Uint16 down;
    Uint16 left;
    Uint16 right;
    Uint16 accept;
    Uint16 cancel;
    Uint16 menu;

    Uint16 acceptA;
    Uint16 cancelA;
    Uint16 menuA;

    Uint16 acceptB;
    Uint16 cancelB;
    Uint16 menuB;
} KeyBinds;

/* TODO controller bindings */

KeyBinds GetDefaultKeyBinds(void);
void PressVInput(Uint32* vPad, InputButton b);
void UnpressVInput(Uint32* vPad, InputButton b);
/* NOTE: Does not accept aliases, only standard pressed or held */
SDL_bool CheckVInput(Uint32 vPad, InputButton b);

#endif /* INPUT_H */

/* implementation */
#ifdef INPUT_C

#include "bitflag.c"

KeyBinds GetDefaultKeyBinds(void)
{
    return (KeyBinds){
        .up = SDLK_UP, .down = SDLK_DOWN, .left = SDLK_LEFT, .right = SDLK_RIGHT, .accept = SDLK_z, .cancel = SDLK_x, .menu = SDLK_c,
        .acceptA = SDLK_RETURN, .cancelA = SDLK_RSHIFT, .menuA = SDLK_RCTRL,
        .acceptB = SDLK_KP_ENTER, .cancelB = SDLK_LSHIFT, .menuB = SDLK_LCTRL,
    };
}

void PressVInput(Uint32* vPad, InputButton b)
{
    SetFlag(vPad, (Uint32)b);
}

void UnpressVInput(Uint32* vPad, InputButton b)
{
    ClearFlag(vPad, (Uint32)b);
}

SDL_bool CheckVInput(Uint32 vPad, InputButton b)
{
    InputButton ba = 0, bb = 0;
    switch (b)
    {
        case VKEY_UP:
        case VKEY_DOWN:
        case VKEY_LEFT:
        case VKEY_RIGHT:
        case VKEY_UP_HELD:
        case VKEY_DOWN_HELD:
        case VKEY_LEFT_HELD:
        case VKEY_RIGHT_HELD:
            /* no aliases */
        break;

        case VKEY_ACCEPT:
            ba = VKEY_ACCEPT_A;
            bb = VKEY_ACCEPT_B;
        break;

        case VKEY_CANCEL:
            ba = VKEY_CANCEL_A;
            bb = VKEY_CANCEL_B;
        break;

        case VKEY_MENU:
            ba = VKEY_MENU_A;
            bb = VKEY_MENU_B;
        break;

        case VKEY_ACCEPT_HELD:
            ba = VKEY_ACCEPT_A_HELD;
            bb = VKEY_ACCEPT_B_HELD;
        break;

        case VKEY_CANCEL_HELD:
            ba = VKEY_CANCEL_A_HELD;
            bb = VKEY_CANCEL_B_HELD;
        break;

        case VKEY_MENU_HELD:
            ba = VKEY_MENU_A_HELD;
            bb = VKEY_MENU_B_HELD;
        break;

        default:
            printf("WARNING: CheckVInput: Passed unsupported input type %i\n", b);
        break;
    }

    return CheckFlag(vPad, b) || CheckFlag(vPad, ba) || CheckFlag(vPad, bb);
}

#endif /* INPUT_C */
