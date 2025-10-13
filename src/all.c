#ifndef ALL_C
#define ALL_C

#include "statusflag.h"
#define PLAYER_C
#include "player.c"
#define ROOM_C
#include "room.c"
#define TEXTBOX_C
#include "textbox.c"
#define TEXTDRAW_C
#include "textdraw.c"
#define INPUT_C
#include "input.c"
#define BITFLAG_C
#include "bitflag.c"
#define TYPES_C
#include "types.c"
#define UTILS_C
#include "utils.c"

#define SDL_STBIMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include "SDL_stbimage.h"

#endif /* ALL_C */
