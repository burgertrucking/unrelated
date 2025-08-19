#include "statusflags.h"

void SetFlag(Uint8* flags, StatusFlag val)
{
    *flags |= val;
}

void ClearFlag(Uint8* flags, StatusFlag val)
{
    *flags &= ~val;
}

void ToggleFlag(Uint8* flags, StatusFlag val)
{
    *flags ^= val;
}

SDL_bool CheckFlag(Uint8 flags, StatusFlag val)
{
    return (flags & val) != 0;
}
