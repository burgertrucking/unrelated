#include "SDL/SDL.h"
#include "game.h"
#include "includeall.c"

/* NOTE: hot reloading not yet implemented, need to modify makefile to support it */
#ifdef ENABLE_HOT_RELOADING
	#include "hotreload.h"
#else
	#include "game.c"
#endif

int main(int argc, char** argv)
{
	#ifdef ENABLE_HOT_RELOADING
        int err = LoadGameDll();
        if (err) return err;
	    printf("Hot reloading enabled, press F5 to re-load game dll\n");
	#endif

	/* init */
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
	SDL_WM_SetCaption("Test Window", NULL);
	GameState state;
	#ifdef ENABLE_HOT_RELOADING
		gameDll.InitGame(&state);
	#else
		InitGame(&state);
	#endif

	/* update + draw */
	while (!CheckFlag(state.statusFlags, STATUS_QUIT))
	{
		#ifdef ENABLE_HOT_RELOADING
			gameDll.UpdateDrawFrame(&state);
			if (CheckFlag(state.statusFlags, STATUS_HOT_RELOAD))
			{
				HandleHotReload();
				ClearFlag(&state.statusFlags, STATUS_HOT_RELOAD);
			}
		#else
			UpdateDrawFrame(&state);
		#endif
	}
	
	/* quit */
	SDL_Quit();

	return 0;
}
