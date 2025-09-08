#ifndef STATUSFLAG_H
#define STATUSFLAG_H

typedef enum StatusFlag
{
    STATUS_QUIT = 1 << 0,
    STATUS_HOT_RELOAD = 1 << 1, /* does nothing when ENABLE_HOT_RELOAD is undefined */
    STATUS_WINDOW_RESIZED = 1 << 2,
    STATUS_FULLSCREEN = 1 << 3,
} StatusFlag;
#endif /* STATUSFLAG_H */
