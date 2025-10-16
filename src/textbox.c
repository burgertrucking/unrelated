/* header */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "SDL.h"
#include "textdraw.c"

typedef struct Textbox
{
    SDL_Surface* graphic;
    String msg; /* TEMP these should be stored elsewhere */
    unsigned int charsDrawn;
    SDL_bool shouldDraw;
} Textbox;

int InitTextbox(Textbox* tb);
void UpdateTextbox(Textbox* tb, Uint32 vPad, Uint32* status);
/* expects vscreen480 */
int DrawTextbox(Textbox* tb, SDL_bool isDW, Fonts* fnt, SDL_Surface* screen);

#endif /* TEXTBOX_H */

/* implementation */
#ifdef TEXTBOX_C

#include "types.c"
#include "bitflag.c"
#include "input.c"
#include "statusflag.h"

int InitTextbox(Textbox* tb)
{
    tb->graphic = LoadImage("temp-textbox.png");
    if (!tb->graphic) return 1;
    tb->charsDrawn = 0;
    tb->shouldDraw = SDL_FALSE;
    tb->msg = (String){ "* \"Try your best, Astral Wolf!\"\n* \"Even in your darkest hour...!\"", 66 };
    return 0;
}

void UpdateTextbox(Textbox* tb, Uint32 vPad, Uint32* status)
{
    if (tb->shouldDraw)
    {
        if (tb->charsDrawn >= tb->msg.len)
        {
            /* TODO add press C to skip text */
            if (CheckFlag(vPad, VKEY_ACCEPT) || CheckFlag(vPad, VKEY_ACCEPT_A) || CheckFlag(vPad, VKEY_ACCEPT_B))
            {
                tb->shouldDraw = SDL_FALSE;
                ClearFlag(status, STATUS_IS_CUTSCENE);
                tb->charsDrawn = 0;
            }
        }
        else
        {
            /* STUB */
            if (CheckFlag(vPad, VKEY_CANCEL) || CheckFlag(vPad, VKEY_CANCEL_A) || CheckFlag(vPad, VKEY_CANCEL_B))
                tb->charsDrawn = tb->msg.len;
            else ++tb->charsDrawn;
        }
    }
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
        String msgSlice = tb->msg;
        msgSlice.len = tb->charsDrawn;
        err = DrawText(msgSlice, fnt, screen, font, textStart);
    }
    return err;
}

#endif /* TEXTBOX_C */
