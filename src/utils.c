/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"

/* Currently just wraps IMG_Load */
SDL_Surface* LoadImage(const char* file);
/* Convenience wrapper for blitting a surface at a given point */
int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y);

#endif

/* implementation */
#ifdef UTILS_C

#include "SDL_image.h"

SDL_Surface* LoadImage(const char* file)
{
    return IMG_Load(file);
}

int BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* dst, int x, int y)
{
    SDL_Rect dstRect = (SDL_Rect){ x, y, 0, 0 };
    return SDL_BlitSurface(src, srcRect, dst, &dstRect);
}

#endif
