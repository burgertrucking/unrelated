/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"
#include "SDL_video.h"

/* Load a png image and save it to an SDL_Surface with the same format as the screen */
SDL_Surface* LoadImage(const char* file);
/* Convenience wrapper for blitting a surface at a given point */
int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y);
/* Scale a surface then blit it */
int BlitSurfaceScaled(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y, float scaleX, float scaleY);

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

int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y)
{
    SDL_Rect dstRect = (SDL_Rect){ x, y, 0, 0 };
    return SDL_BlitSurface(src, srcRect, dst, &dstRect);
}

int BlitSurfaceScaled(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y, float scaleX, float scaleY)
{
    SDL_Rect dims;
    dims = srcRect? *srcRect : src->clip_rect;
    Uint16 w = dims.w*scaleX, h = dims.h*scaleY;
    /* TEMP dimension validation: will force an incorrect but working stretch if invalid */
    if ((x + w) > dst->w)
    {
        w = dst->w - x;
        fprintf(stderr, "WARNING: BlitSurfaceScaled: Requested stretch width %i too wide, cutting to %i\n", x+w, w);
    }
    if ((y + h) > dst->h)
    {
        h = dst->h - y;
        fprintf(stderr, "WARNING: BlitSurfaceScaled: Requested stretch height %i too tall, cutting to %i\n", y+h, h);
    }
    SDL_Rect dstRect = (SDL_Rect){ x, y, w, h };
    /* NOTE using internal api since 1.2 doesn't have a SDL_BlitSurfaceScaled() equivalent */
    return SDL_SoftStretch(src, srcRect, dst, &dstRect);
}

#endif
