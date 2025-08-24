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
} InputButton;

void PressVInput(Uint8* vPad, InputButton b);
void UnpressVInput(Uint8* vPad, InputButton b);

#endif /* INPUT_H */

/* implementation */
#ifdef INPUT_C

#include "bitflag.c"

void PressVInput(Uint8* vPad, InputButton b)
{
    SetFlag(vPad, (Uint8)b);
}

void UnpressVInput(Uint8* vPad, InputButton b)
{
    ClearFlag(vPad, (Uint8)b);
}

#endif /* INPUT_C */
