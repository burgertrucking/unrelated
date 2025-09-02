/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"

/* Currently just wraps IMG_Load */
SDL_Surface* LoadImage(const char* file);
/* Inteded for blitting onto the screen */
void BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* screen, int x, int y);

#endif

/* implementation */
#ifdef UTILS_C

#include "SDL_image.h"

SDL_Surface* LoadImage(const char* file)
{
    return IMG_Load(file);
}

void BlitSurfaceCoords(SDL_Surface* src, SDL_Rect* srcRect, SDL_Surface* screen, int x, int y)
{
    SDL_Rect dstRect = (SDL_Rect){ x, y, 0, 0 };
    SDL_BlitSurface(src, srcRect, screen, &dstRect);
}

#endif
