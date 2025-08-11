#include "SDL/SDL.h"

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE);
	SDL_WM_SetCaption("Test Window", NULL);
	SDL_Delay(2000);
	SDL_Quit();
	return 0;
}
