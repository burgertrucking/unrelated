/* header */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "SDL.h"
#include "textdraw.c"

typedef struct Textbox
{
    SDL_Surface* graphic;
    SDL_bool shouldDraw;
} Textbox;

int InitTextbox(Textbox* tb);
/* expects vscreen480 */
int DrawTextbox(Textbox* tb, SDL_bool isDW, Fonts* fnt, SDL_Surface* screen);

#endif /* TEXTBOX_H */

/* implementation */
#ifdef TEXTBOX_C

#include "types.c"

int InitTextbox(Textbox* tb)
{
    tb->graphic = LoadImage("temp-textbox.png");
    if (!tb->graphic) return 1;
    tb->shouldDraw = SDL_FALSE;
    return 0;
}

int DrawTextbox(Textbox* tb, SDL_bool isDW, Fonts* fnt, SDL_Surface* screen)
{
    /* STUB draw textbox test */
    int err = 0;
    if (tb->shouldDraw)
    {
        int font = (isDW)? FONT_MAIN_DW : FONT_MAIN_LW;
        int tbrecty = (isDW)? 0 : 167;
        SDL_Rect tbrect = (SDL_Rect){ 0, tbrecty, 593, 167 };
        err = BlitSurfaceCoords(tb->graphic, &tbrect, screen, (Vec2){24, 312});
        const Vec2 textStart = (Vec2){29*2, 170*2}; /* start position for textboxes */
        const char msg[] = "* \"Try your best, Astral Wolf!\"\n* \"Even in your darkest hour...!\"";
        err = DrawText(msg, fnt, screen, font, textStart);
    }
    return err;
}

#endif /* TEXTBOX_C */
