/* header */
#ifndef UTILS_H
#define UTILS_H

#include "SDL.h"

/* Currently just wraps IMG_Load */
SDL_Surface* LoadImage(const char* file);

#endif

/* implementation */
#ifdef UTILS_C

#include "SDL_image.h"

SDL_Surface* LoadImage(const char* file)
{
    return IMG_Load(file);
}

#endif
