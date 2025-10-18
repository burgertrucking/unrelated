/* header */
#ifndef TEXTDRAW_H
#define TEXTDRAW_H

#include "SDL.h"
#include "types.c"

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

/* Draw a message at a given position */
int DrawText(String msg, Fonts* fontImgs, SDL_Surface* screen, FontType font, Vec2 pos);

#endif

/* implementation */
#ifdef TEXTDRAW_C

#include <stdio.h>
#include "utils.c"

enum
{
    TEXT_CHAR_WIDTH = 17,
    TEXT_CHAR_HEIGHT = 33,

    FONT_IMG_ROWS = 9,
    FONT_IMG_COLS = 30,

    TEXT_CHAR_SPACING_DEFAULT = TEXT_CHAR_WIDTH - 1,
    TEXT_LINE_SPACING_DEFAULT = TEXT_CHAR_HEIGHT + 3,
};

/* Calculate the rectangle that clips a given character from the bitmap font */
static SDL_Rect GetCharRect(const char letter);

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

int DrawText(String msg, Fonts* fontImgs, SDL_Surface* screen, FontType font, Vec2 pos)
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

    const int startX = pos.x - TEXT_CHAR_SPACING_DEFAULT;
    unsigned int i;
    Vec2 charPos;
    /* null character check allows for rendering strings shorter than their buffer size (eg. when drawing fps) */
    /* TODO log when null character check goes off */
    for (i = 0, charPos.x = startX, charPos.y = pos.y; i < msg.len && msg.data[i] != '\0'; ++i)
    {
        if (msg.data[i] != '\n')
        {
            SDL_Rect srcrect = GetCharRect(msg.data[i]);
            charPos.x += TEXT_CHAR_SPACING_DEFAULT;
            BlitSurfaceCoords(f, &srcrect, screen, charPos);
        }
        else
        {
            charPos.x = startX;
            charPos.y += TEXT_LINE_SPACING_DEFAULT;
        }
    }

    return 0;
};

static SDL_Rect GetCharRect(const char letter)
{
    int col = letter%FONT_IMG_COLS;
    int row = (letter - col)/FONT_IMG_COLS;
    return (SDL_Rect){ col*TEXT_CHAR_WIDTH, row*TEXT_CHAR_HEIGHT, TEXT_CHAR_WIDTH, TEXT_CHAR_HEIGHT };
}

#endif
