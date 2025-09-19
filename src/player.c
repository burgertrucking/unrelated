/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"

typedef struct Player {
    SDL_Surface* sprite;
    float x;
    float y;
    unsigned int frameCount;
    unsigned int animFrame;
    unsigned int facing;
} Player;

int InitPlayer(Player* p);
void UpdatePlayer(Player* p, Uint32 vPad);
int DrawPlayer(Player* p, SDL_Surface* screen);

#endif /* PLAYER_H */

/* implementation */
#ifdef PLAYER_C

/* constants */
enum
{
    PLAYER_SPRITE_WIDTH = 19,
    PLAYER_SPRITE_HEIGHT = 38,

    PLAYER_FACE_DOWN = 0,
    PLAYER_FACE_LEFT,
    PLAYER_FACE_RIGHT,
    PLAYER_FACE_UP,

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
    p->animFrame = 0;
    p->facing = PLAYER_FACE_DOWN;
    p->x = 0.0f;
    p->y = 0.0f;

    return 0;
}

void UpdatePlayer(Player* p, Uint32 vPad)
{
    SDL_bool isRunning = CheckFlag(vPad, VKEY_CANCEL);
    int moveSpeed = isRunning? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
    /* TODO make turning act like it does in the real game, where you maintain direction unless 180ing */
    if (CheckFlag(vPad, VKEY_DOWN))
    {
        p->y += moveSpeed;
        p->facing = PLAYER_FACE_DOWN;
    }
    if (CheckFlag(vPad, VKEY_UP))
    {
        p->y -= moveSpeed;
        p->facing = PLAYER_FACE_UP;
    }
    if (CheckFlag(vPad, VKEY_RIGHT))
    {
        p->x += moveSpeed;
        p->facing = PLAYER_FACE_RIGHT;
    }
    if (CheckFlag(vPad, VKEY_LEFT))
    {
        p->x -= moveSpeed;
        p->facing = PLAYER_FACE_LEFT;
    }

    /* TODO play animations depending on whether or not you're moving */
    int animFps = isRunning? PLAYER_RUN_FPS : PLAYER_WALK_FPS;
    ++p->frameCount;
    /* HACK using TICKS_PER_SECOND without proper import from game.c, import properly */
    if (p->frameCount >= TICKS_PER_SECOND/animFps)
    {
        p->frameCount = 0;
        ++p->animFrame;
        if (p->animFrame >= 4) p->animFrame = 0;
    }
}

int DrawPlayer(Player* p, SDL_Surface* screen)
{
    SDL_Rect srcRect = (SDL_Rect){
        p->animFrame*PLAYER_SPRITE_WIDTH, p->facing*PLAYER_SPRITE_HEIGHT,
        PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT
    };
    return BlitSurfaceCoords(p->sprite, &srcRect, screen, p->x, p->y);
}

#endif /* PLAYER_C */
