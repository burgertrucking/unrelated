#include "SDL/SDL.h"
#include "game.h"

/* NOTE: hot reloading not yet implemented, need to modify makefile to support it */
#ifdef ENABLE_HOT_RELOADING
	#include "hotreload.h"
#endif

int main(int argc, char** argv)
{
	#ifdef ENABLE_HOT_RELOADING
	    printf("Hot reloading enabled, press F5 to re-load game dll");
        int err = LoadGameDll();
        if (err)
            return err;
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
	while (!state.shouldQuit)
	{
		#ifdef ENABLE_HOT_RELOADING
			gameDll.UpdateDrawFrame(&state);
		#else
			UpdateDrawFrame(&state);
		#endif
	}
	
	/* quit */
	SDL_Quit();

	return 0;
}
