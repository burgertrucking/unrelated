/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"
#include "types.h"
#include "room.c"

typedef struct Player {
	SDL_Surface* lwSprite;
	SDL_Surface* dwSprite;
	Rect bbox;
	Vec2 pos;
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
void UpdatePlayer(Player* p, Room* room, Uint32 vPad);
int DrawPlayer(Player* p, SDL_Surface* screen);
/* Should be drawn into vscreen240 */
int DrawPlayerGizmos(Player* p, SDL_Surface* screen);

#endif /* PLAYER_H */

/* implementation */
#ifdef PLAYER_C

/* constants */
enum
{
    PLAYER_SPRITE_WIDTH = 19,
    PLAYER_SPRITE_HEIGHT = 38,

/*
	p.BBox = rl.NewRectangle(p.Pos.X, p.Pos.Y + PlayerBBoxYOffset, PlayerSpriteWidth, PlayerBBoxHeight)

	PlayerSpriteWidth = 20
	PlayerSpriteHeight = 31 // FIXME this is just for rendering, "actual" player height is 30
	PlayerBBoxHeight = 14
	PlayerBBoxYOffset = 18 - 1 // TEMP the -1 is because the sprite height is different than the original game
*/
	/* TEMP these are rough approximations */
    PLAYER_BBOX_WIDTH = PLAYER_SPRITE_WIDTH,
	PLAYER_BBOX_HEIGHT = 14,
	PLAYER_BBOX_Y_OFFSET = 25, /* TEMP not sure if this is calculated right */

	/* TODO change to down, right, up, left */
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
    p->dwSprite = LoadImage("res/rip/spr/mainchara-dw.png");
    if (!p->lwSprite || !p->dwSprite)
    {
        fprintf(stderr, "InitPlayer: Could not load sprites\n");
        return 1;
    }
    p->animFrame = 0;
    p->facing = PLAYER_FACE_DOWN;
    p->pos = (Vec2){0};
    p->bbox = (Rect){ p->pos.x, p->pos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
    p->runCount = 0;
    p->isDarkWorld = SDL_FALSE;

    return 0;
}

void UpdatePlayer(Player* p, Room* room, Uint32 vPad)
{
    /* handle inputs */
    /* TODO make alias checking less verbose */
    p->isRunning = (CheckFlag(vPad, VKEY_CANCEL) || CheckFlag(vPad, VKEY_CANCEL_A) || CheckFlag(vPad, VKEY_CANCEL_B));
    int moveSpeed;
    if (p->isRunning)
    {
    	if (p->runCount < 10) moveSpeed = (p->isDarkWorld)? PLAYER_RUN_START_SPEED_DW : PLAYER_RUN_START_SPEED_LW;
    	else if (p->runCount > 60) moveSpeed = (p->isDarkWorld)? PLAYER_RUN_LONG_SPEED_DW : PLAYER_RUN_LONG_SPEED_LW;
    	else moveSpeed = (p->isDarkWorld)? PLAYER_RUN_SPEED_DW : PLAYER_RUN_SPEED_LW;
    }
    else moveSpeed = (p->isDarkWorld)? PLAYER_WALK_SPEED_DW : PLAYER_WALK_SPEED_LW;
    Vec2 dir = (Vec2){0};
    if (CheckFlag(vPad, VKEY_DOWN) || CheckFlag(vPad, VKEY_DOWN_A) || CheckFlag(vPad, VKEY_DOWN_B)) dir.y = 1;
    if (CheckFlag(vPad, VKEY_UP) || CheckFlag(vPad, VKEY_UP_A) || CheckFlag(vPad, VKEY_UP_B)) dir.y = -1;
    if (CheckFlag(vPad, VKEY_RIGHT) || CheckFlag(vPad, VKEY_RIGHT_A) || CheckFlag(vPad, VKEY_RIGHT_B)) dir.x = 1;
    if (CheckFlag(vPad, VKEY_LEFT) || CheckFlag(vPad, VKEY_LEFT_A) || CheckFlag(vPad, VKEY_LEFT_B)) dir.x = -1;

    /* handle movement */
    SDL_bool isMoving = SDL_FALSE;
    /* TEMP temporarily just not moving when encountering a collision */
    /* TODO sliding, moving you right up against wall when hitting collision */
    Vec2 newPos = (Vec2){ p->pos.x + dir.x*moveSpeed, p->pos.y + dir.y*moveSpeed };
    Rect newBbox = (Rect){ newPos.x, newPos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
    int i;
    SDL_bool hit = SDL_FALSE;
    for (i = 0; i < room->wallsLen; ++i)
    {
	    if (CheckAABBCollision(newBbox, room->walls[i]))
	    {
	    	hit = SDL_TRUE;
	    	break;
	    }
    }
    if (!hit)
    {
    	p->pos = newPos;
    	p->bbox = newBbox;
    	if (dir.x != 0 || dir.y != 0) isMoving = SDL_TRUE;
    }

    /* handle turning */
	switch (p->facing)
	{
		case PLAYER_FACE_DOWN:
			if (dir.y < 0) p->facing = PLAYER_FACE_UP;
			else if (dir.y == 0)
			{
				if (dir.x < 0) p->facing = PLAYER_FACE_LEFT;
				else if (dir.x > 0) p->facing = PLAYER_FACE_RIGHT;
			}
		break;
		case PLAYER_FACE_UP:
			if (dir.y > 0) p->facing = PLAYER_FACE_DOWN;
			else if (dir.y == 0)
			{
				if (dir.x < 0) p->facing = PLAYER_FACE_LEFT;
				else if (dir.x > 0) p->facing = PLAYER_FACE_RIGHT;
			}
		break;
		case PLAYER_FACE_RIGHT:
			if (dir.x < 0) p->facing = PLAYER_FACE_LEFT;
			else if (dir.x == 0)
			{
				if (dir.y < 0) p->facing = PLAYER_FACE_UP;
				else if (dir.y > 0) p->facing = PLAYER_FACE_DOWN;
			}
    	break;
		case PLAYER_FACE_LEFT:
			if (dir.x > 0) p->facing = PLAYER_FACE_RIGHT;
			else if (dir.x == 0)
			{
				if (dir.y < 0) p->facing = PLAYER_FACE_UP;
				else if (dir.y > 0) p->facing = PLAYER_FACE_DOWN;
			}
		break;
	}

	/* handle movement of bbox */
	/* TEMP */
    p->bbox = (Rect){ p->pos.x, p->pos.y + PLAYER_BBOX_Y_OFFSET,
    				  PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };

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
    int err = BlitSurfaceCoords(sprite, &srcRect, screen, p->pos);
    return err;
}

int DrawPlayerGizmos(Player* p, SDL_Surface* screen)
{
    /* TEMP creating rectangle of bbox */
    SDL_Rect bboxGfx = (SDL_Rect){ p->bbox.x, p->bbox.y, p->bbox.w, p->bbox.h };
    int err = SDL_FillRect(screen, &bboxGfx, SDL_MapRGB(screen->format, 0, 255, 0));
    return err;
}

#endif /* PLAYER_C */
