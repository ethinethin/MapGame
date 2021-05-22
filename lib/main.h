#ifndef MAIN_H
#define MAIN_H

#define LOADBAR // turn on loading bar

#include <SDL2/SDL.h>
#include "maps.h"

/* Main game structure */
struct game {
	SDL_bool running;
	struct {
		int w;
		int h;
		float scale_x;
		float scale_y;
		SDL_bool vsync;
		int displaymode;
		SDL_bool scanlines_on;
		SDL_bool display_reloaded;
		const char *name;
		SDL_Window *window;
		SDL_Renderer *renderer;
		SDL_Texture *output;
	} screen;
	SDL_Texture **font;
	SDL_Texture **sprite_textures;
	SDL_Texture *map_texture;
	SDL_Texture *scanlines;
	SDL_Texture **craft;
	char cursor;
	SDL_bool inventory;
};

/* Map dimensions */
#define MAP_ROWS 512
#define MAP_COLS 1024
#define WIN_ROWS 11
#define WIN_COLS 19
#define WIN_SCALE 2
#define SPRITE_W 32
#define SPRITE_H 32

/* Screen dimensions values */
#define WIN_W 1280
#define WIN_H 720
#define GAME_X 32
#define GAME_Y 8
#define GAME_W 1216
#define GAME_H 704
#define MAP_X 128
#define MAP_Y 104
#define MAP_W 1024
#define MAP_H 512

/* Constants for inventory and quickbar */
#define INV_W (SPRITE_W * WIN_SCALE * 4 + 1)
//#define INV_H ((SPRITE_H * WIN_SCALE + (SPRITE_H * WIN_SCALE / 4)) * 8 + 1)
#define INV_H (SPRITE_H * WIN_SCALE * 1.25 * 8 + 1)
#define INV_X (WIN_W - GAME_X - 16 - INV_W)
#define INV_Y (WIN_H - GAME_Y - 16 - INV_H)
#define QB_X (WIN_W/2 - (SPRITE_W * WIN_SCALE*8)/2)
#define QB_Y (WIN_H - GAME_Y - 16 - (SPRITE_H * WIN_SCALE * 1.25))
#define QB_W (SPRITE_W * WIN_SCALE * 8)
#define QB_H (SPRITE_H * WIN_SCALE * 1.25)
/* Constants for holders */
#define HOLDER_W (SPRITE_W * WIN_SCALE * 10)
#define HOLDER_H (SPRITE_H * WIN_SCALE * 4 * 1.25)
#define HOLDER_X QB_X + QB_W - HOLDER_W
#define HOLDER_Y QB_Y - HOLDER_H - SPRITE_H * WIN_SCALE * 1.25
/* Constants for making */
#define MAKER_W (SPRITE_W * WIN_SCALE * 8)
#define MAKER_H (SPRITE_H * WIN_SCALE * 1.25 * 6)
#define MAKER_X (WIN_W / 2 - MAKER_W / 2)
#define MAKER_Y (WIN_H - GAME_Y - 16 - INV_H)

/* Constants for loot */
#define MAX_STACK 999

#endif
