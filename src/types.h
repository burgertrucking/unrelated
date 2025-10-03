#ifndef TYPES_H
#define TYPES_H

/* TODO: Modify game.c and utils.c to use these instead of hardcoded xywh values */

/* Distinct from SDL_Rect in that its fields are floats */
typedef struct Rect
{
    float x;
    float y;
    float w;
    float h;
} Rect;

typedef struct Vec2
{
    float x;
    float y;
} Vec2;

#endif /* TYPES_H */
