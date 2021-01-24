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
#define MAP_ROWS 512
#define MAP_COLS 1024
#define WIN_ROWS 21
#define WIN_COLS 39
#define SPRITE_W 32
#define SPRITE_H 32

/* Screen dimensions values */
#define WIN_W 1280
#define WIN_H 720
#define GAME_X 16
#define GAME_Y 24
#define GAME_W 1248
#define GAME_H 672
#define MAP_X 128
#define MAP_Y 104
#define MAP_W 1024
#define MAP_H 512

#endif
