/* header */
#ifndef INPUT_H
#define INPUT_H

#include "SDL.h"

typedef enum InputButton
{
    VKEY_UP = 1 << 0,
    VKEY_DOWN = 1 << 1,
    VKEY_LEFT = 1 << 2,
    VKEY_RIGHT = 1 << 3,
    VKEY_ACCEPT = 1 << 4,
    VKEY_CANCEL = 1 << 5,
    VKEY_MENU = 1 << 6,

    /* Each button gets two extra aliases (eg. Enter, Shift, Control) like in the original.
    Unlike the original, so do the movement keys. */
    VKEY_UP_A = 1 << 7,
    VKEY_DOWN_A = 1 << 8,
    VKEY_LEFT_A = 1 << 9,
    VKEY_RIGHT_A = 1 << 10,
    VKEY_ACCEPT_A = 1 << 11,
    VKEY_CANCEL_A = 1 << 12,
    VKEY_MENU_A = 1 << 13,

    VKEY_UP_B = 1 << 14,
    VKEY_DOWN_B = 1 << 15,
    VKEY_LEFT_B = 1 << 16,
    VKEY_RIGHT_B = 1 << 17,
    VKEY_ACCEPT_B = 1 << 18,
    VKEY_CANCEL_B = 1 << 19,
    VKEY_MENU_B = 1 << 20,
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

    Uint16 upA;
    Uint16 downA;
    Uint16 leftA;
    Uint16 rightA;
    Uint16 acceptA;
    Uint16 cancelA;
    Uint16 menuA;

    Uint16 upB;
    Uint16 downB;
    Uint16 leftB;
    Uint16 rightB;
    Uint16 acceptB;
    Uint16 cancelB;
    Uint16 menuB;
} KeyBinds;

/* TODO controller bindings */

KeyBinds GetDefaultKeyBinds(void);
void PressVInput(Uint32* vPad, InputButton b);
void UnpressVInput(Uint32* vPad, InputButton b);

#endif /* INPUT_H */

/* implementation */
#ifdef INPUT_C

#include "bitflag.c"

KeyBinds GetDefaultKeyBinds(void)
{
    return (KeyBinds){
        .up = SDLK_UP, .down = SDLK_DOWN, .left = SDLK_LEFT, .right = SDLK_RIGHT, .accept = SDLK_z, .cancel = SDLK_x, .menu = SDLK_c,
        .upA = SDLK_w, .downA = SDLK_s, .leftA = SDLK_a, .rightA = SDLK_d, .acceptA = SDLK_RETURN, .cancelA = SDLK_RSHIFT, .menuA = SDLK_RCTRL,
        .upB = SDLK_i, .downB = SDLK_k, .leftB = SDLK_j, .rightB = SDLK_l, .acceptB = SDLK_KP_ENTER, .cancelB = SDLK_LSHIFT, .menuB = SDLK_LCTRL,
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

#endif /* INPUT_C */
