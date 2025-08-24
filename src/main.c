#include "SDL.h"

#ifdef ENABLE_HOT_RELOADING
	#define HOTRELOAD_C
	#include "hotreload.c"
	#include "game.c"
	#include "statusflag.h"
	#define BITFLAG_C
	#include "bitflag.c"
#else
	#define GAME_STANDALONE /* include the implementation in a static build */
	#include "game.c"
#endif /* ENABLE_HOT_RELOADING */

int main(int argc, char* argv[])
{
	int err = 0;
	#ifdef ENABLE_HOT_RELOADING
        err = LoadGameDll();
        if (err) return err;
	    printf("Hot reloading enabled, press F5 to re-load game dll\n");
	#endif

	/* init */
	err = SDL_Init(SDL_INIT_EVERYTHING);
	if (err) return err;
	GameState state;
	#ifdef ENABLE_HOT_RELOADING
		err = gameDll.InitGame(&state);
		if (err) return err;
	#else
		err = InitGame(&state);
		if (err) return err;
	#endif

	/* update + draw */
	while (!CheckFlag(state.statusFlags, STATUS_QUIT))
	{
		#ifdef ENABLE_HOT_RELOADING
			err = gameDll.UpdateDrawFrame(&state);
			if (err) return err;
			if (CheckFlag(state.statusFlags, STATUS_HOT_RELOAD))
			{
				HandleHotReload();
				ClearFlag(&state.statusFlags, STATUS_HOT_RELOAD);
			}
		#else
			err = UpdateDrawFrame(&state);
			if (err) return err;
		#endif
	}
	
	/* quit */
	SDL_Quit();

	return err;
}
