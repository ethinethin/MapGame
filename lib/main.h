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
	SDL_Texture **sprite_textures;
	SDL_Surface **font;
	char cursor;
	SDL_bool inventory;
	SDL_bool fullscreen;
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

/* Constants for inventory and quickbar */
#define INV_X (WIN_W - GAME_X - 16 - 192)
#define INV_Y (GAME_Y + 16 + 18)
#define INV_W (48*4 + 1)
#define INV_H (60*8 + 1)
#define QB_X (WIN_W/2 - 48*8/2)
#define QB_Y (WIN_H - GAME_Y - 60 - 16)
#define QB_W (48*8)
#define QB_H 60


#endif
