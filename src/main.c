#include "SDL.h"

#ifdef ENABLE_HOT_RELOADING
	#define HOTRELOAD_IMPL
	#include "hotreload.c"
	#include "game.c"
	#define HIMPL
	#include "statusflags.c"
#else
	#define GAME_STANDALONE /* include the implementation in a static build */
	#include "game.c"
#endif /* ENABLE_HOT_RELOADING */

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
