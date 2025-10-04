/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"
#include "types.h"

/* Load a png image and save it to an SDL_Surface with the same format as the screen */
SDL_Surface* LoadImage(const char* file);
/* Convenience wrapper for blitting a surface at a given point */
int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, Vec2 pos);
/* Scale a surface then blit it. Does not alpha blend */
int BlitSurfaceScaled(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, Vec2 pos, Vec2 scale);
SDL_bool CheckAABBCollision(Rect a, Rect b);

#endif

/* implementation */
#ifdef UTILS_C

#include "SDL_stbimage.h"

SDL_Surface* LoadImage(const char* file)
{
    SDL_Surface* orig = STBIMG_Load(file); /* raw loaded image */
    SDL_Surface* native = SDL_DisplayFormatAlpha(orig); /* native to screen format */
    SDL_FreeSurface(orig);
    return native;
}

int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, Vec2 pos)
{
    SDL_Rect dstRect = (SDL_Rect){ pos.x, pos.y, 0, 0 };
    return SDL_BlitSurface(src, srcRect, dst, &dstRect);
}

int BlitSurfaceScaled(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, Vec2 pos, Vec2 scale)
{
    SDL_Rect dims;
    dims = (srcRect)? *srcRect : src->clip_rect;
    Uint16 w = dims.w*scale.x, h = dims.h*scale.y;
    /* TEMP dimension validation: will force an incorrect but working stretch if invalid */
    if ((pos.x + w) > dst->w)
    {
        w = dst->w - pos.x;
        fprintf(stderr, "WARNING: BlitSurfaceScaled: Requested stretch width %f too wide, cutting to %i\n", pos.x+w, w);
    }
    if ((pos.y + h) > dst->h)
    {
        h = dst->h - pos.y;
        fprintf(stderr, "WARNING: BlitSurfaceScaled: Requested stretch height %f too tall, cutting to %i\n", pos.y+h, h);
    }
    SDL_Rect dstRect = (SDL_Rect){ pos.x, pos.y, w, h };
    /* NOTE using internal api since 1.2 doesn't have a SDL_BlitSurfaceScaled() equivalent */
    return SDL_SoftStretch(src, srcRect, dst, &dstRect);
}

SDL_bool CheckAABBCollision(Rect a, Rect b)
{
    return
        (a.x + a.w) >= b.x &&
        a.x <= (b.x + b.w) &&
        (a.y + a.h) >= b.y &&
        a.y <= (b.y + b.h)
    ;
}

#endif
