/* header */
#ifndef PLAYER_H
#define PLAYER_H

#include "SDL.h"
#include "types.c"
#include "room.c"
#include "textbox.c"

typedef struct Player {
	SDL_Surface* lwSprite;
	SDL_Surface* dwSprite;
	Rect bbox;
    Rect checkBbox;
	Vec2 pos;
    unsigned int runCount; /* counts frames moved for acceleration */
	unsigned int stillCount; /* counts consecutive frames player stands still; used to play animations while tap moving (not exact to original) */
    unsigned int frameCount;
    unsigned int animFrame;
    unsigned int facing;
	 /* TODO move to a bitflag with other bools */
    SDL_bool isRunning;
} Player;

int InitPlayer(Player* p);
void UpdatePlayer(Player* p, Room* room, Textbox* tb, Uint32 vPad, Uint32* status);
int DrawPlayer(Player* p, SDL_Surface* screen, Uint32 status);
/* Should be drawn into vscreen240 */
int DrawPlayerGizmos(Player* p, SDL_Surface* screen);

#endif /* PLAYER_H */

/* implementation */
#ifdef PLAYER_C

#include <stdio.h>
#include "utils.c"
#include "input.c"
#include "bitflag.c"

/* constants */
enum
{
    PLAYER_SPRITE_WIDTH = 19,
    PLAYER_SPRITE_HEIGHT = 38,

    PLAYER_BBOX_WIDTH = PLAYER_SPRITE_WIDTH,
    PLAYER_BBOX_HEIGHT = 14,
    PLAYER_BBOX_Y_OFFSET = 25,

    PLAYER_CHK_BBOX_X_OFFSET_VER = 4,
    PLAYER_CHK_BBOX_X_OFFSET_RIGHT = PLAYER_SPRITE_WIDTH/2,
    PLAYER_CHK_BBOX_X_OFFSET_LEFT = -15,
    PLAYER_CHK_BBOX_Y_OFFSET_HOR = PLAYER_BBOX_Y_OFFSET,
    PLAYER_CHK_BBOX_Y_OFFSET_DOWN = 28,
    PLAYER_CHK_BBOX_Y_OFFSET_UP = 5,
    PLAYER_CHK_BBOX_WIDTH_VER = PLAYER_SPRITE_WIDTH - 8,
    PLAYER_CHK_BBOX_HEIGHT_VER = PLAYER_SPRITE_HEIGHT - 11,
    PLAYER_CHK_BBOX_WIDTH_HOR = PLAYER_SPRITE_WIDTH/2 + 15,
    PLAYER_CHK_BBOX_HEIGHT_HOR = PLAYER_BBOX_HEIGHT,

    PLAYER_FACE_DOWN = 0,
    PLAYER_FACE_RIGHT,
    PLAYER_FACE_UP,
    PLAYER_FACE_LEFT,

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

static Rect calcCheckBbox(Player* p);

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
    p->pos = (Vec2){ 260, 110 }; /* TEMP these are hardcoded for the classroom */
    p->bbox = (Rect){ p->pos.x, p->pos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
    p->checkBbox = calcCheckBbox(p);
    p->runCount = 0;
    return 0;
}

void UpdatePlayer(Player* p, Room* room, Textbox* tb, Uint32 vPad, Uint32* status)
{
	SDL_bool isCutscene = CheckFlag(*status, STATUS_IS_CUTSCENE);
	SDL_bool isDarkWorld = CheckFlag(*status, STATUS_IS_DARK_WORLD);
	/* STUB just don't update player when in cutscene */
	/* consider using fsm instead for this */
	if (isCutscene) return;

    /* handle inputs */
    p->isRunning = CheckVInput(vPad, VKEY_CANCEL_HELD);
    int moveSpeed;
    if (p->isRunning)
    {
    	if (p->runCount < 10) moveSpeed = (isDarkWorld)? PLAYER_RUN_START_SPEED_DW : PLAYER_RUN_START_SPEED_LW;
    	else if (p->runCount > 60) moveSpeed = (isDarkWorld)? PLAYER_RUN_LONG_SPEED_DW : PLAYER_RUN_LONG_SPEED_LW;
    	else moveSpeed = (isDarkWorld)? PLAYER_RUN_SPEED_DW : PLAYER_RUN_SPEED_LW;
    }
    else moveSpeed = (isDarkWorld)? PLAYER_WALK_SPEED_DW : PLAYER_WALK_SPEED_LW;
    Vec2 dir = (Vec2){0};
    if (CheckVInput(vPad, VKEY_DOWN_HELD)) dir.y = 1;
    if (CheckVInput(vPad, VKEY_UP_HELD)) dir.y = -1;
    if (CheckVInput(vPad, VKEY_RIGHT_HELD)) dir.x = 1;
    if (CheckVInput(vPad, VKEY_LEFT_HELD)) dir.x = -1;

    /* handle movement */
    SDL_bool isMoving = SDL_FALSE;
    Vec2 newPos = Vec2Add(p->pos, Vec2Scale(dir, moveSpeed));
    Rect newBboxX = (Rect){ newPos.x, p->pos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
    Rect newBboxY = (Rect){ p->pos.x, newPos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
    int i;
    for (i = 0; i < room->wallsLen; ++i)
    {
		SDL_bool hitX = RectCheckCollisions(newBboxX, room->walls[i]);
		SDL_bool hitY = RectCheckCollisions(newBboxY, room->walls[i]);
		if (hitX || hitY)
		{
			if (hitX)
		    {
			if (dir.x < 0) newPos.x = room->walls[i].x + room->walls[i].w;
			else if (dir.x > 0) newPos.x = room->walls[i].x - p->bbox.w;
		    }
		    if (hitY)
			{
				if (dir.y < 0) newPos.y = room->walls[i].y + room->walls[i].h - PLAYER_BBOX_Y_OFFSET;
				else if (dir.y > 0) newPos.y = room->walls[i].y - p->bbox.h - PLAYER_BBOX_Y_OFFSET;
		    }
		}
		/* edge case for corner collisions that aren't detected when axes are split */
		/* unused because corner teleportation is fun */
		/*
	    else
	    {
		    Rect newBbox = (Rect){ newPos.x, newPos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };
			if (RectCheckCollisions(newBbox, room->walls[i]))
		    {
				switch (p->facing)
				{
					case PLAYER_FACE_DOWN:
					case PLAYER_FACE_UP:
						newPos.x = p->pos.x;
					break;
					case PLAYER_FACE_RIGHT:
					case PLAYER_FACE_LEFT:
						newPos.y = p->pos.y;
					break;
				}
			}
		}
		*/
    }
	if (!Vec2Equals(p->pos, newPos))
	{
		p->pos = newPos;
		isMoving = SDL_TRUE;
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
    p->bbox = (Rect){ p->pos.x, p->pos.y + PLAYER_BBOX_Y_OFFSET, PLAYER_BBOX_WIDTH, PLAYER_BBOX_HEIGHT };

    /* handle movement of check bbox */
    p->checkBbox = calcCheckBbox(p);
    /* handle checking */
    for (i = 0; i < room->interactablesLen; ++i)
    {
        Interactable* obj = &room->interactables[i];
        if (RectCheckCollisions(p->checkBbox, obj->bbox))
        {
			if (CheckVInput(vPad, VKEY_ACCEPT))
	        {
	        	SetFlag(status, STATUS_IS_CUTSCENE);
	        	p->animFrame = 0;
	        	tb->shouldDraw = SDL_TRUE;
	        	if (obj->msgsLen == 0)
	        	{
	        		printf("WARNING: UpdatePlayer: Interactable %i has no messages, falling back to default\n", i);
	        		tb->msgToDraw = -1;
	        	}
	        	else
	        	{
	                tb->msgToDraw = obj->msgs[obj->checkCount];
	                SDL_bool maxChecks = obj->checkCount >= obj->msgsLen - 1;
	                if (maxChecks) printf("DEBUG: UpdatePlayer: max checks (%i) on interactable %i\n", obj->msgsLen, i);
	                else ++obj->checkCount;
	        	}
	        	isMoving = SDL_FALSE;
	        }
	    }
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

int DrawPlayer(Player* p, SDL_Surface* screen, Uint32 status)
{
	SDL_bool isDarkWorld = CheckFlag(status, STATUS_IS_DARK_WORLD);
    SDL_Rect srcRect = (SDL_Rect){
        p->animFrame*PLAYER_SPRITE_WIDTH, p->facing*PLAYER_SPRITE_HEIGHT,
        PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT
    };
    /* TEMP this probably should not be reassigned each frame */
    SDL_Surface* sprite = (isDarkWorld)? p->dwSprite : p->lwSprite;
    int err = BlitSurfaceCoords(sprite, &srcRect, screen, p->pos);
    return err;
}

int DrawPlayerGizmos(Player* p, SDL_Surface* screen)
{
    /* TEMP creating rectangle of bbox */
    SDL_Rect bboxGfx = (SDL_Rect){ p->bbox.x, p->bbox.y, p->bbox.w, p->bbox.h };
    SDL_Rect checkGfx = (SDL_Rect){ p->checkBbox.x, p->checkBbox.y, p->checkBbox.w, p->checkBbox.h };
    int err = SDL_FillRect(screen, &bboxGfx, SDL_MapRGB(screen->format, 0, 255, 0));
    err = SDL_FillRect(screen, &checkGfx, SDL_MapRGB(screen->format, 0, 255, 255));
    return err;
}

static Rect calcCheckBbox(Player* p)
{
    Rect c;
    switch (p->facing)
    {
        case PLAYER_FACE_DOWN:
            c.x = p->pos.x + PLAYER_CHK_BBOX_X_OFFSET_VER;
            c.y = p->pos.y + PLAYER_CHK_BBOX_Y_OFFSET_DOWN;
            c.w = PLAYER_CHK_BBOX_WIDTH_VER;
            c.h = PLAYER_CHK_BBOX_HEIGHT_VER;
        break;
        case PLAYER_FACE_RIGHT:
            c.x = p->pos.x + PLAYER_CHK_BBOX_X_OFFSET_RIGHT;
            c.y = p->pos.y + PLAYER_CHK_BBOX_Y_OFFSET_HOR;
            c.w = PLAYER_CHK_BBOX_WIDTH_HOR;
            c.h = PLAYER_CHK_BBOX_HEIGHT_HOR;
        break;
        case PLAYER_FACE_UP:
            c.x = p->pos.x + PLAYER_CHK_BBOX_X_OFFSET_VER;
            c.y = p->pos.y + PLAYER_CHK_BBOX_Y_OFFSET_UP;
            c.w = PLAYER_CHK_BBOX_WIDTH_VER;
            c.h = PLAYER_CHK_BBOX_HEIGHT_VER;
        break;
        case PLAYER_FACE_LEFT:
            c.x = p->pos.x + PLAYER_CHK_BBOX_X_OFFSET_LEFT;
            c.y = p->pos.y + PLAYER_CHK_BBOX_Y_OFFSET_HOR;
            c.w = PLAYER_CHK_BBOX_WIDTH_HOR;
            c.h = PLAYER_CHK_BBOX_HEIGHT_HOR;
        break;
    }
    return c;
}

#endif /* PLAYER_C */
