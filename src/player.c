/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"

typedef struct Player {
    SDL_Surface* sprite;
    Sint16 x;
    Sint16 y;
} Player;

int InitPlayer(Player* p);
void UpdatePlayer(Player* p, Uint8 vPad);
void DrawPlayer(Player* p, SDL_Surface* screen);

#endif /* PLAYER_H */

/* implementation */
#ifdef PLAYER_C

static const Sint16 playerSpeed = 3;

#include <stdio.h>
#include "SDL_image.h"
#include "input.c"
#include "bitflag.c"

int InitPlayer(Player* p)
{
    p->sprite = IMG_Load("res/edit/img/mainchara.png");
    if (!p->sprite)
    {
        fprintf(stderr, "InitPlayer: Could not load sprite\n");
        return 1;
    }
    p->x = 0;
    p->y = 0;

    return 0;
}

void UpdatePlayer(Player* p, Uint8 vPad)
{
    if (CheckFlag(vPad, VKEY_DOWN)) p->y += playerSpeed;
    if (CheckFlag(vPad, VKEY_UP)) p->y -= playerSpeed;
    if (CheckFlag(vPad, VKEY_RIGHT)) p->x += playerSpeed;
    if (CheckFlag(vPad, VKEY_LEFT)) p->x -= playerSpeed;
}

void DrawPlayer(Player* p, SDL_Surface* screen)
{
    SDL_Rect drawRect = (SDL_Rect){ p->x, p->y, 0, 0 };
    SDL_BlitSurface(p->sprite, NULL, screen, &drawRect);
}

#endif /* PLAYER_C */
