#ifdef _WIN32
    #include <windef.h> /* data types (needed for WinMain) */
#endif /* _WIN32 */
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

int logError(int err, const char* callerFn);

/* TODO figure out sdl_main to circumvent the need for this */
int main(int argc, char* argv[])
/* int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) */
{
	int err = 0;
	#ifdef ENABLE_HOT_RELOADING
        err = LoadGameDll();
        if (err) return err; /* error message propagation done in LoadGameDll(), just need to return value */
	    printf("Hot reloading enabled, press F5 to re-load game dll\n");
	#endif

	/* init */
	err = SDL_Init(SDL_INIT_EVERYTHING);
	if (err) return logError(err, "SDL_Init in main");
	GameState state;
	#ifdef ENABLE_HOT_RELOADING
		err = gameDll.InitGame(&state);
	#else
		err = InitGame(&state);
	#endif
	if (err) logError(err, "InitGame");

	/* update + draw */
	while (!CheckFlag(state.statusFlags, STATUS_QUIT))
	{
		#ifdef ENABLE_HOT_RELOADING
			err = gameDll.UpdateDrawFrame(&state);
			if (err) logError(err, "UpdateDrawFrame");
			if (CheckFlag(state.statusFlags, STATUS_HOT_RELOAD))
			{
				HandleHotReload(); /* this function logs errors, no need to propagate its return value */
				ClearFlag(&state.statusFlags, STATUS_HOT_RELOAD);
			}
		#else
			err = UpdateDrawFrame(&state);
			if (err) logError(err, "UpdateDrawFrame");
		#endif
	}
	
	/* quit */
	SDL_Quit();

	return err;
}

int logError(int err, const char* callerFn)
{
	fprintf(stderr, "WARNING: %s returned error: `%s`\n", callerFn, SDL_GetError());
	return err;
}
