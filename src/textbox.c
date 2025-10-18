/* header */
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "SDL.h"
#include "textdraw.c"
#include "room.c"

typedef struct Textbox
{
    SDL_Surface* graphic;
    unsigned int charsDrawn;
    SDL_bool shouldDraw;
    int msgToDraw; /* value of -1 indicates an erroneous message index */
} Textbox;

int InitTextbox(Textbox* tb);
void UpdateTextbox(Textbox* tb, RoomMessage* msgs, Uint32 vPad, Uint32* status);
/* expects vscreen480 */
int DrawTextbox(Textbox* tb, RoomMessage* msgs, SDL_bool isDW, Fonts* fnt, SDL_Surface* screen);

#endif /* TEXTBOX_H */

/* implementation */
#ifdef TEXTBOX_C

#include "types.c"
#include "bitflag.c"
#include "input.c"
#include "statusflag.h"

static const String errorMsg = (String){ "You FUCKED up the room messages\nIDIOT", 69 };

int InitTextbox(Textbox* tb)
{
    tb->graphic = LoadImage("temp-textbox.png");
    if (!tb->graphic) return 1;
    tb->charsDrawn = 0;
    tb->shouldDraw = SDL_FALSE;
    return 0;
}

void UpdateTextbox(Textbox* tb, RoomMessage* msgs, Uint32 vPad, Uint32* status)
{
    if (tb->shouldDraw)
    {
        String msg;
        int next;
        if (tb->msgToDraw == -1)
        {
            msg = errorMsg;
            next = -1;
        }
        else
        {
            msg = msgs[tb->msgToDraw].msg;
            next = msgs[tb->msgToDraw].next;
        }
        if (tb->charsDrawn >= msg.len)
        {
            /* TODO add press C to skip text */
            if (CheckFlag(vPad, VKEY_ACCEPT) || CheckFlag(vPad, VKEY_ACCEPT_A) || CheckFlag(vPad, VKEY_ACCEPT_B)
                || CheckFlag(vPad, VKEY_MENU_HELD) || CheckFlag(vPad, VKEY_MENU_A_HELD) || CheckFlag(vPad, VKEY_MENU_B_HELD))
            {
                if (next >= 0) tb->msgToDraw = next;
                else
                {
                    tb->shouldDraw = SDL_FALSE;
                    ClearFlag(status, STATUS_IS_CUTSCENE);
                }
                tb->charsDrawn = 0;
            }
        }
        else
        {
            /* STUB */
            if (CheckFlag(vPad, VKEY_CANCEL) || CheckFlag(vPad, VKEY_CANCEL_A) || CheckFlag(vPad, VKEY_CANCEL_B)
                || CheckFlag(vPad, VKEY_MENU_HELD) || CheckFlag(vPad, VKEY_MENU_A_HELD) || CheckFlag(vPad, VKEY_MENU_B_HELD))
                tb->charsDrawn = msg.len;
            else ++tb->charsDrawn; /* intentionally increment to 1 when first rendering, otherwise box will be blank */
        }
    }
}

int DrawTextbox(Textbox* tb, RoomMessage* msgs, SDL_bool isDW, Fonts* fnt, SDL_Surface* screen)
{
    int err = 0;
    if (tb->shouldDraw)
    {
        String msgSlice;
        if (tb->msgToDraw == -1) msgSlice = errorMsg;
        else msgSlice = msgs[tb->msgToDraw].msg;
        int font = (isDW)? FONT_MAIN_DW : FONT_MAIN_LW;
        int tbrecty = (isDW)? 0 : 167;
        SDL_Rect tbrect = (SDL_Rect){ 0, tbrecty, 593, 167 };
        err = BlitSurfaceCoords(tb->graphic, &tbrect, screen, (Vec2){24, 312});
        const Vec2 textStart = (Vec2){29*2, 170*2}; /* start position for textboxes */
        msgSlice.len = tb->charsDrawn;
        err = DrawText(msgSlice, fnt, screen, font, textStart);
    }
    return err;
}

#endif /* TEXTBOX_C */
