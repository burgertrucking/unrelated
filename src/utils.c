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
    /* NOTE dimension validation is handled by SDL_SoftStretch() rather than here */
    SDL_Rect dstRect = (SDL_Rect){ x, y, src->w*scaleX, src->h*scaleY };
    /* NOTE using internal api since 1.2 doesn't have a SDL_BlitSurfaceScaled() equivalent */
    return SDL_SoftStretch(src, srcRect, dst, &dstRect);
}

#endif
