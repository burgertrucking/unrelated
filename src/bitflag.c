/* header */
#ifndef BITFLAG_H
#define BITFLAG_H

#include "SDL.h"

void SetFlag(Uint8* flags, Uint8 val);
void ClearFlag(Uint8* flags, Uint8 val);
void ToggleFlag(Uint8* flags, Uint8 val);
SDL_bool CheckFlag(Uint8 flags, Uint8 val);

#endif /* BITFLAG_H */

/* implementation */
#ifdef BITFLAG_C

#include "SDL.h"

void SetFlag(Uint8* flags, Uint8 val)
{
    *flags |= val;
}

void ClearFlag(Uint8* flags, Uint8 val)
{
    *flags &= ~val;
}

void ToggleFlag(Uint8* flags, Uint8 val)
{
    *flags ^= val;
}

SDL_bool CheckFlag(Uint8 flags, Uint8 val)
{
    return (flags & val) != 0;
}

#endif /* BITFLAG_C */
