/* header */
#ifndef BITFLAG_H
#define BITFLAG_H

#include "SDL.h"

/* TEMP macro version */
/*
#define SetFlag(F, V) *F |= V
#define ClearFlag(F, V) *F &= ~V
#define ToggleFlag(F, V) *F ^= V
#define CheckFlag(F, V) ((F & V) != 0)
*/

/* function version */
void SetFlag(Uint32* flags, Uint32 val);
void ClearFlag(Uint32* flags, Uint32 val);
void ToggleFlag(Uint32* flags, Uint32 val);
SDL_bool CheckFlag(Uint32 flags, Uint32 val);

#endif /* BITFLAG_H */

/* implementation */
#ifdef BITFLAG_C

#include "SDL.h"

void SetFlag(Uint32* flags, Uint32 val)
{
    *flags |= val;
}

void ClearFlag(Uint32* flags, Uint32 val)
{
    *flags &= ~val;
}

void ToggleFlag(Uint32* flags, Uint32 val)
{
    *flags ^= val;
}

SDL_bool CheckFlag(Uint32 flags, Uint32 val)
{
    return (flags & val) != 0;
}

#endif /* BITFLAG_C */
