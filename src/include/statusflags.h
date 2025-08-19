#ifndef STATUSFLAGS_H
#define STATUSFLAGS_H

#include "SDL/SDL.h"

typedef enum StatusFlag
{
    STATUS_QUIT = 1 << 0,
    STATUS_HOT_RELOAD = 1 << 1, /* does nothing when ENABLE_HOT_RELOAD is undefined */
} StatusFlag;

void SetFlag(Uint8* flags, StatusFlag val);
void ClearFlag(Uint8* flags, StatusFlag val);
void ToggleFlag(Uint8* flags, StatusFlag val);
SDL_bool CheckFlag(Uint8 flags, StatusFlag val);

#endif /* STATUSFLAGS_H */
