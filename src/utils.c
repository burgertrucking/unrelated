/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"

/* Load a png image and save it to an SDL_Surface with the same format as the screen */
SDL_Surface* LoadImage(const char* file);
/* Convenience wrapper for blitting a surface at a given point */
int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y);
/* Scale a surface then blit it */
int BlitSurfaceScaled(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y, float scaleX, float scaleY);

#endif

/* implementation */
#ifdef UTILS_C

#include "SDL_image.h"

SDL_Surface* LoadImage(const char* file)
{
    SDL_Surface* orig = IMG_Load(file); /* raw loaded image */
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
    SDL_Rect dstRect = (SDL_Rect){ x, y, x*scaleX, x*scaleY };
    /* HACK using internal api since 1.2 doesn't have a SDL_BlitSurfaceScaled() equivalent */
    /* TEMP using NULL for dstRec since I'm currently unsure what the expected parameter is */
    return SDL_SoftStretch(src, srcRect, dst, NULL);
}

#endif
