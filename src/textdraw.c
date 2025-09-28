/* header */
#ifndef TEXTDRAW_H
#define TEXTDRAW_H

#include "SDL.h"

/* struct for bitmap fonts */
typedef struct Fonts
{
    SDL_Surface* lwMain;
    SDL_Surface* dwMain;
} Fonts;

typedef enum FontType
{
    FONT_MAIN_LW,
    FONT_MAIN_DW,
} FontType;

int InitFonts(Fonts* fonts);

/* STUB just draws the font image at the given position */
int DrawText(Fonts* fontImgs, SDL_Surface* screen, FontType font, int x, int y);

#endif

/* implementation */
#ifdef TEXTDRAW_C

#include <stdio.h>
#include "utils.c"

int InitFonts(Fonts* fonts)
{
    fonts->lwMain = LoadImage("res/rip/fnt/maintext-lw.png");
    fonts->dwMain = LoadImage("res/rip/fnt/maintext-dw.png");
    if (!fonts->lwMain || !fonts->dwMain)
    {
        fprintf(stderr, "InitFonts: Could not load bitmap font files\n");
        return 1;
    }
    return 0;
}

int DrawText(Fonts* fontImgs, SDL_Surface* screen, FontType font, int x, int y)
{
    SDL_Surface* f;
    switch (font)
    {
        case FONT_MAIN_LW:
            f = fontImgs->lwMain;
        break;

        case FONT_MAIN_DW:
            f = fontImgs->dwMain;
        break;

        default:
            printf("WARNING: DrawText: Invalid fonttype %i specified, returning early\n", font);
            return 1;
        break;
    }

    /* TEMP */
    BlitSurfaceCoords(f, NULL, screen, x, y);

    return 0;
};

#endif
