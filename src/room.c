/* header */
#ifndef ROOM_H
#define ROOM_H

#include "SDL.h"
#include "types.c"

enum
{
    TILE_SIZE = 20,
    MAX_WALLS = 64,
};

typedef struct Room
{
    Rect walls[MAX_WALLS];
    int wallsLen;
} Room;

int DrawRoomGizmos(Room* r, SDL_Surface* screen);

#endif /* ROOM_H */

/* implementation */
#ifdef ROOM_C

int DrawRoomGizmos(Room* r, SDL_Surface* screen)
{
    int i, err = 0;
    for (i = 0; i < r->wallsLen; ++i)
    {
        /* TEMP draw solid blue rectangles the size of each wall */
        SDL_Rect bboxGfx = (SDL_Rect){ r->walls[i].x, r->walls[i].y, r->walls[i].w, r->walls[i].h };
        err = SDL_FillRect(screen, &bboxGfx, SDL_MapRGB(screen->format, 0, 0, 255));
        if (err) return err;
    }
    return err;
}

#endif /* ROOM_C */
