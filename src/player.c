/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"

typedef struct Player {
    SDL_Surface* sprite;
    unsigned int animFrames;
    Sint16 x;
    Sint16 y;
} Player;

int InitPlayer(Player* p);
void UpdatePlayer(Player* p, Uint8 vPad);
void DrawPlayer(Player* p, SDL_Surface* screen);

#endif /* PLAYER_H */

/* implementation */
#ifdef PLAYER_C

/* constants */
enum
{
    PLAYER_WALK_SPEED = 3,
    PLAYER_RUN_SPEED = 5, /* TEMP UTY values as placeholders until accelerated running is implemented */
    PLAYER_WALK_FPS = 5,
    PLAYER_RUN_FPS = 7,
};

#include <stdio.h>
#include "utils.c"
#include "input.c"
#include "bitflag.c"

int InitPlayer(Player* p)
{
    p->sprite = LoadImage("res/edit/spr/mainchara.png");
    if (!p->sprite)
    {
        fprintf(stderr, "InitPlayer: Could not load sprite\n");
        return 1;
    }
    p->animFrames = 0;
    p->x = 0;
    p->y = 0;

    return 0;
}

void UpdatePlayer(Player* p, Uint8 vPad)
{
    int moveSpeed = (CheckFlag(vPad, VKEY_CANCEL))? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
    if (CheckFlag(vPad, VKEY_DOWN)) p->y += moveSpeed;
    if (CheckFlag(vPad, VKEY_UP)) p->y -= moveSpeed;
    if (CheckFlag(vPad, VKEY_RIGHT)) p->x += moveSpeed;
    if (CheckFlag(vPad, VKEY_LEFT)) p->x -= moveSpeed;

    ++p->animFrames;
}

void DrawPlayer(Player* p, SDL_Surface* screen)
{
    SDL_Rect drawRect = (SDL_Rect){ p->x, p->y, 0, 0 };
    SDL_BlitSurface(p->sprite, NULL, screen, &drawRect);
}

#endif /* PLAYER_C */
