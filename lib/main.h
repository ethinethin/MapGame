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
	char **seen;
};

/* Map dimensions */
#define MAP_ROWS 256
#define MAP_COLS 256
#define WIN_ROWS 19
#define WIN_COLS 19
#define SPRITE_W 32
#define SPRITE_H 32

/* Screen dimensions values */
#define WIN_W 1170
#define WIN_H 640
#define GAME_X 16
#define GAME_Y 16
#define GAME_W 608
#define GAME_H 608
#define MAP_X 640
#define MAP_Y 16
#define MAP_W 514
#define MAP_H 514

#endif
