/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"

typedef struct Player {
	/* NOTE consider combining these into one big spreadsheet */
	SDL_Surface* lwSprite;
	SDL_Surface* dwSprite;
    float x;
    float y;
    unsigned int runCount; /* counts frames moved for acceleration */
	unsigned int stillCount; /* counts consecutive frames player stands still; used to play animations while tap moving (not exact to original) */
    unsigned int frameCount;
    unsigned int animFrame;
    unsigned int facing;
	 /* TODO move to a bitflag with other bools */
    SDL_bool isDarkWorld;
    SDL_bool isRunning;
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

    PLAYER_WALK_SPEED_BASE = 2,
    PLAYER_WALK_SPEED_LW = PLAYER_WALK_SPEED_BASE + 1,
    PLAYER_WALK_SPEED_DW = PLAYER_WALK_SPEED_BASE,
    PLAYER_RUN_START_SPEED_LW = PLAYER_WALK_SPEED_BASE + 2,
    PLAYER_RUN_SPEED_LW = PLAYER_WALK_SPEED_BASE + 3,
    PLAYER_RUN_LONG_SPEED_LW = PLAYER_WALK_SPEED_BASE + 4,
    PLAYER_RUN_START_SPEED_DW = PLAYER_WALK_SPEED_BASE + 1,
    PLAYER_RUN_SPEED_DW = PLAYER_WALK_SPEED_BASE + 2,
    PLAYER_RUN_LONG_SPEED_DW = PLAYER_WALK_SPEED_BASE + 3,

    PLAYER_WALK_FPS = 4,
    PLAYER_RUN_FPS = 8,
};

#include <stdio.h>
#include "utils.c"
#include "input.c"
#include "bitflag.c"

int InitPlayer(Player* p)
{
    p->lwSprite = LoadImage("res/edit/spr/mainchara-lw.png");
    p->dwSprite = LoadImage("res/edit/spr/mainchara-dw.png");
    if (!p->lwSprite || !p->dwSprite)
    {
        fprintf(stderr, "InitPlayer: Could not load sprites\n");
        return 1;
    }
    p->animFrame = 0;
    p->facing = PLAYER_FACE_DOWN;
    p->x = 0.0f;
    p->y = 0.0f;
    p->runCount = 0;
    p->isDarkWorld = SDL_FALSE;

    return 0;
}

void UpdatePlayer(Player* p, Uint32 vPad)
{
    /* handle inputs */
    p->isRunning = CheckFlag(vPad, VKEY_CANCEL);
    int moveSpeed;
    if (p->isRunning)
    {
    	if (p->runCount < 10) moveSpeed = (p->isDarkWorld)? PLAYER_RUN_START_SPEED_DW : PLAYER_RUN_START_SPEED_LW;
    	else if (p->runCount > 60) moveSpeed = (p->isDarkWorld)? PLAYER_RUN_LONG_SPEED_DW : PLAYER_RUN_LONG_SPEED_LW;
    	else moveSpeed = (p->isDarkWorld)? PLAYER_RUN_SPEED_DW : PLAYER_RUN_SPEED_LW;
    }
    else moveSpeed = (p->isDarkWorld)? PLAYER_WALK_SPEED_DW : PLAYER_WALK_SPEED_LW;
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
    if (isMoving)
    {
        int animFps = p->isRunning? PLAYER_RUN_FPS : PLAYER_WALK_FPS;
        ++p->frameCount;
        /* HACK using TICKS_PER_SECOND without proper import from game.c, import properly */
        if (p->frameCount >= TICKS_PER_SECOND/animFps)
        {
            p->frameCount = 0;
            ++p->animFrame;
            if (p->animFrame >= 4) p->animFrame = 0;
        }
        p->stillCount = 0;
        if (p->isRunning) ++p->runCount;
        else p->runCount = 0;
    }
    else if (p->stillCount >= 4)
    {
        p->animFrame = 0;
        p->frameCount = 0;
        p->stillCount = 0;
    }
    else
    {
		++p->stillCount;
    	p->runCount = 0;
    }
}

int DrawPlayer(Player* p, SDL_Surface* screen)
{
    SDL_Rect srcRect = (SDL_Rect){
        p->animFrame*PLAYER_SPRITE_WIDTH, p->facing*PLAYER_SPRITE_HEIGHT,
        PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT
    };
    /* TEMP this probably should not be reassigned each frame */
    SDL_Surface* sprite = (p->isDarkWorld)? p->dwSprite : p->lwSprite;
    return BlitSurfaceCoords(sprite, &srcRect, screen, p->x, p->y);
}

#endif /* PLAYER_C */
