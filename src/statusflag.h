#ifndef STATUSFLAG_H
#define STATUSFLAG_H

typedef enum StatusFlag
{
    STATUS_QUIT = 1 << 0,
    STATUS_HOT_RELOAD = 1 << 1, /* does nothing when ENABLE_HOT_RELOAD is undefined */
    STATUS_WINDOW_RESIZED = 1 << 2,
    STATUS_FULLSCREEN = 1 << 3,
    STATUS_QUIT_KEY_HELD = 1 << 4, /* holding escape but haven't elapsed timer yet */
    STATUS_DRAW_FPS = 1 << 5,
    STATUS_DRAW_GIZMOS = 1 << 6, /* draws various debugging graphics akin to unity gizmos */
} StatusFlag;
#endif /* STATUSFLAG_H */
