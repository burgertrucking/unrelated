/* header */
#ifndef ROOM_H
#define ROOM_H

#include "SDL.h"
#include "types.c"

enum
{
    TILE_SIZE = 20,
    MAX_TILE_DIM = 64, /* square this for total amount of tiles */
    MAX_WALLS = 64,
    MAX_SLOPES = 32,
    MAX_ROOM_MESSAGES = 64,
    MAX_INTERACTABLES = 64,
    MAX_INTERACTABLE_CHECKS = 8,
};

typedef enum RoomSheetType
{
    ROOM_SHEET_TILEMAP, /* the image is the room's tileset (eg. lw school hallway) */
    ROOM_SHEET_WHOLE, /* the image is of the room (eg. lw classrooms) */
} RoomSheetType;

typedef struct Tile
{
    Vec2 sprPos; /* sprite's position in tilemap (unit: pixels) */
} Tile;

typedef struct RoomSheet
{
    SDL_Surface* img;
    RoomSheetType type;
} RoomSheet;

/* defines which corner the slope's right angle is located */
/* (hypotenuse is opposite this point) */
typedef enum SlopeType
{
    SLOPE_TOP_LEFT,
    SLOPE_TOP_RIGHT,
    SLOPE_BOTTOM_LEFT,
    SLOPE_BOTTOM_RIGHT,
} SlopeType;
typedef struct Slope
{
    Vec2 pos;
    SlopeType corner;
} Slope;

typedef struct Interactable
{
    Rect bbox; /* nonsolids have a bbox of (0, 0, 0, 0) */
    int msgs[MAX_INTERACTABLE_CHECKS];
    int msgsLen;
    int checkCount;
} Interactable;

typedef struct RoomMessage
{
    String msg;
    int next; /* -1 means textbox closes after this message */
} RoomMessage;

typedef struct Room
{
    Vec2 size; /* in tiles */
    SDL_Surface* surface; /* all tiles are drawn to this surface upon init */
    SDL_Surface* slopespr;
    RoomSheet sheet;
    RoomMessage msgs[MAX_ROOM_MESSAGES];
    Tile tiles[MAX_TILE_DIM][MAX_TILE_DIM];
    Rect walls[MAX_WALLS];
    Slope slopes[MAX_SLOPES];
    Interactable interactables[MAX_INTERACTABLES];
    int tilesLen;
    int wallsLen;
    int slopesLen;
    int interactablesLen;
    /* NOTE: length of msgs is not currently tracked */
} Room;

int InitRoom(Room* r, const char* sheetFile, RoomSheetType sheetType);
int DrawRoom(Room* r, SDL_Surface* screen);
int DrawRoomGizmos(Room* r, SDL_Surface* screen);

#endif /* ROOM_H */

/* implementation */
#ifdef ROOM_C

#include "utils.c"

int InitRoom(Room* r, const char* sheetFile, RoomSheetType sheetType)
{
    int err = 0;

    r->sheet.img = LoadImage(sheetFile);
    r->slopespr = LoadImage("res-temp/corner-tiles.png");
    r->sheet.type = sheetType;
    if (r->sheet.type == ROOM_SHEET_WHOLE)
    {
        r->surface = r->sheet.img;
        r->tilesLen = 0;
    }
    else if (!r->sheet.img || !r->slopespr)
    {
        err = 1;
    }
    else
    {
        printf("WARNING: InitRoom() sheet type tilemap currently unhandled\n");
        err = 2;
        return err;
    }

    /* TODO handle creating walls and interactables */

    return err;
}

/* can either draw to vscreen240 or vscreen480 depending on room specifics */
int DrawRoom(Room* r, SDL_Surface* screen)
{
    return BlitSurfaceCoords(r->surface, NULL, screen, (Vec2){0});
}

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
    for (i = 0; i < r->interactablesLen; ++i)
    {
        /* TEMP draw solid pink rectangles the size of each interactable */
        SDL_Rect bboxGfx = (SDL_Rect){
            r->interactables[i].bbox.x, r->interactables[i].bbox.y,
            r->interactables[i].bbox.w, r->interactables[i].bbox.h
        };
        err = SDL_FillRect(screen, &bboxGfx, SDL_MapRGB(screen->format, 255, 128, 128));
        if (err) return err;
    }
    for (i = 0; i < r->slopesLen; ++i)
    {
        SDL_Rect ssprRect = (SDL_Rect){ r->slopes[i].corner*TILE_SIZE, 0, TILE_SIZE, TILE_SIZE };
        err = BlitSurfaceCoords(r->slopespr, &ssprRect, screen, r->slopes[i].pos);
        if (err) return err;
    }

    return err;
}

#endif /* ROOM_C */
