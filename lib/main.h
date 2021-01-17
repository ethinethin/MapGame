#ifndef MAIN_H
#define MAIN_H

#include <SDL2/SDL.h>
#include "maps.h"

/* Main game structure */
struct game {
	SDL_bool running;
	struct {
		unsigned int w;
		unsigned int h;
		const char *name;
		SDL_Window *window;
		SDL_Renderer *renderer;
	} screen;
	SDL_Surface **sprites;
	SDL_bool fullscreen;
};

/* Main player structure */
struct player {
	int x;
	int y;
};

/* Map dimensions */
#define MAP_ROWS 500
#define MAP_COLS 500
#define WIN_ROWS 19
#define WIN_COLS 19
#define SPRITE_W 32
#define SPRITE_H 32

#endif

