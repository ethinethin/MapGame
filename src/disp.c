#include <stdio.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "main.h"
#include "maps.h"

/* Structure for window position */
struct win_pos {
	int x;
	int y;
};

/* Function prototypes */
static void			draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index);
static void			draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win);
static struct win_pos	find_win_pos(struct worldmap *map, struct player *cur_player);

void
display_init(struct game *cur_game)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL error: %s\n", SDL_GetError());
		exit(1);
	}

	/* Create the main window and renderer */
	cur_game->screen.window = SDL_CreateWindow(
		cur_game->screen.name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		cur_game->screen.w, cur_game->screen.h, 0
	);
	cur_game->screen.renderer = SDL_CreateRenderer(
		cur_game->screen.window, -1,
		SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC
	);
}

void
display_quit(struct game *cur_game)
{	
	SDL_DestroyRenderer(cur_game->screen.renderer);
	cur_game->screen.renderer = NULL;
	SDL_DestroyWindow(cur_game->screen.window);
	cur_game->screen.window = NULL;
	SDL_Quit();
}

void
draw_rect(struct game *cur_game, unsigned int x, unsigned int y, unsigned int w, unsigned int h, SDL_bool fill, int *fill_col, SDL_bool border, int *bord_col)
{
	SDL_Rect coords = { x, y, w, h };
	SDL_SetRenderDrawColor(cur_game->screen.renderer, fill_col[0], fill_col[1], fill_col[2], 255);
	
	/* Draw filled rectangle? */
	if (fill) {
		SDL_RenderFillRect(cur_game->screen.renderer, &coords);
	} else {
		SDL_RenderDrawRect(cur_game->screen.renderer, &coords);
	}
	
	/* Draw border? */
	if (border && bord_col != NULL) {
		SDL_SetRenderDrawColor(cur_game->screen.renderer, bord_col[0], bord_col[1], bord_col[2], 0xFF);
		SDL_RenderDrawRect(cur_game->screen.renderer, &coords);
	}
}

static void
draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index)
{
	SDL_Rect rect = {x, y, w, h};
	SDL_Texture* texture = SDL_CreateTextureFromSurface(cur_game->screen.renderer, cur_game->sprites[sprite_index]);
	SDL_RenderCopy(cur_game->screen.renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

void
draw_map(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int x, y;
	int sprite_index;
	struct win_pos win;

	/* Update window position */
	win = find_win_pos(map, cur_player);
	
	/* draw map */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	for (y = win.y; y < win.y+WIN_ROWS; y++) {
		for (x = win.x; x < win.x+WIN_COLS; x++) {
			switch (*(*(map->tile+y)+x)) {
				case 1:
					sprite_index = 5;
					break;
				case 2:
					sprite_index = 12;
					break;
				case 3:
					sprite_index = 2;
					break;
				case 4:
					sprite_index = 143;
					break;
				case 5:
					sprite_index = 31;
					break;
				default:
					sprite_index = 0;
					break;
			}
			draw_tile(cur_game, (x - win.x)*SPRITE_W, (y - win.y)*SPRITE_H, SPRITE_W, SPRITE_H, sprite_index); 
		}
	}
	draw_player(cur_game, cur_player, win);
	SDL_RenderPresent(cur_game->screen.renderer);
}

static void
draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win)
{
	draw_tile(cur_game,
		  cur_player->x*SPRITE_W - win.x*SPRITE_W,
		  cur_player->y*SPRITE_H - win.y*SPRITE_H,
		  SPRITE_W, SPRITE_H, 328); 
}

void
load_sprites(struct game *cur_game)
{
	/* Load the map arrow sprites */
	SDL_Surface* surface = SDL_LoadBMP("art/sprites.bmp");
	cur_game->sprites = (SDL_Surface**) malloc(sizeof(SDL_Surface*)*512);
	int i, j;
	SDL_Rect rect = {0, 0, SPRITE_W, SPRITE_H};
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 32; j++) {
			cur_game->sprites[(i*32)+j] = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 24, 0x00, 0x00, 0x00, 0x00);
			SDL_SetColorKey(cur_game->sprites[(i*32)+j], 1, 0x000000);
			SDL_FillRect(cur_game->sprites[(i*32)+j], 0, 0x000000);
			rect.x = j * 32;
			rect.y = i * 32;
			SDL_BlitSurface(surface, &rect, cur_game->sprites[(i*32)+j], NULL);
		}
	}
	SDL_FreeSurface(surface);
}

static struct win_pos
find_win_pos(struct worldmap *map, struct player *cur_player)
{
	struct win_pos win;
	win.x = cur_player->x - WIN_COLS/2;
	win.y = cur_player->y - WIN_ROWS/2;
	
	/* correct x value */
	if (win.x < 0) {
		win.x = 0;
	} else if (win.x > MAP_COLS - WIN_COLS) {
		win.x = MAP_COLS - WIN_COLS;
	}
	
	/* correct y value */
	if (win.y < 0) {
		win.y = 0;
	} else if (win.y > MAP_ROWS - WIN_ROWS) {
		win.y = MAP_ROWS - WIN_ROWS;
	}
	
	return win;
}
