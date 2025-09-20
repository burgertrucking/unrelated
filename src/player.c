/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"
#include "SDL_stdinc.h"

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

    PLAYER_WALK_SPEED = 2,
    PLAYER_RUN_SPEED = 4, /* TODO accelerated running */
    PLAYER_WALK_FPS = 4,
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
    /* handle inputs */
    SDL_bool isRunning = CheckFlag(vPad, VKEY_CANCEL);
    int moveSpeed = isRunning? PLAYER_RUN_SPEED : PLAYER_WALK_SPEED;
    int xDir = 0, yDir = 0;
    if (CheckFlag(vPad, VKEY_DOWN)) yDir = 1;
    if (CheckFlag(vPad, VKEY_UP)) yDir = -1;
    if (CheckFlag(vPad, VKEY_RIGHT)) xDir = 1;
    if (CheckFlag(vPad, VKEY_LEFT)) xDir = -1;

    /* handle movement */
    SDL_bool isMoving = (xDir != 0 || yDir != 0)? SDL_TRUE : SDL_FALSE;
    /* STUB collision calculations before moving would go here */
    p->x += xDir*moveSpeed;
    p->y += yDir*moveSpeed;

    /* handle turning */
	switch (p->facing)
	{
		case PLAYER_FACE_DOWN:
			if (yDir < 0) p->facing = PLAYER_FACE_UP;
			else if (yDir == 0)
			{
				if (xDir < 0) p->facing = PLAYER_FACE_LEFT;
				else if (xDir > 0) p->facing = PLAYER_FACE_RIGHT;
			}
		break;
		case PLAYER_FACE_UP:
			if (yDir > 0) p->facing = PLAYER_FACE_DOWN;
			else if (yDir == 0)
			{
				if (xDir < 0) p->facing = PLAYER_FACE_LEFT;
				else if (xDir > 0) p->facing = PLAYER_FACE_RIGHT;
			}
		break;
		case PLAYER_FACE_RIGHT:
			if (xDir < 0) p->facing = PLAYER_FACE_LEFT;
			else if (xDir == 0)
			{
				if (yDir < 0) p->facing = PLAYER_FACE_UP;
				else if (yDir > 0) p->facing = PLAYER_FACE_DOWN;
			}
    	break;
		case PLAYER_FACE_LEFT:
			if (xDir > 0) p->facing = PLAYER_FACE_RIGHT;
			else if (xDir == 0)
			{
				if (yDir < 0) p->facing = PLAYER_FACE_UP;
				else if (yDir > 0) p->facing = PLAYER_FACE_DOWN;
			}
		break;
	}

	/* handle animations */
	/* TODO turn some of these magic numbers into named constants */
	static int stillCount; /* counts consecutive frames player stands still; used to play animations while tap moving (not exact to original) */
    if (isMoving)
    {
        int animFps = isRunning? PLAYER_RUN_FPS : PLAYER_WALK_FPS;
        ++p->frameCount;
        /* HACK using TICKS_PER_SECOND without proper import from game.c, import properly */
        if (p->frameCount >= TICKS_PER_SECOND/animFps)
        {
            p->frameCount = 0;
            ++p->animFrame;
            if (p->animFrame >= 4) p->animFrame = 0;
        }
        stillCount = 0;
    }
    else if (stillCount >= 4)
    {
        p->animFrame = 0;
        p->frameCount = 0;
        stillCount = 0;
    }
    else ++stillCount;
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
