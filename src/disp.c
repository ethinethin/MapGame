#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

/* Structure for window position */
struct win_pos {
	int x;
	int y;
};

/* Function prototypes */
static void		draw_point(struct game *cur_game, int x, int y, char *col);
static void		draw_line(struct game *cur_game, int x1, int y1, int x2, int y2, char *col);
static void		draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win);
static void		draw_inv(struct game *cur_game, struct player *cur_player);
static void		draw_map(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static struct win_pos	find_win_pos(struct worldmap *map, struct player *cur_player);
static void		load_sprites(struct game *cur_game);
static void		unload_sprites(struct game *cur_game);

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
		SDL_RENDERER_ACCELERATED
	);
	SDL_SetRenderDrawBlendMode(cur_game->screen.renderer, SDL_BLENDMODE_BLEND);
	
	/* Load sprites and font */
	load_sprites(cur_game);
	load_font(cur_game);
	
	/* Clear screen */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	SDL_RenderPresent(cur_game->screen.renderer);
	SDL_Event event;
	SDL_WaitEvent(&event);
}

void
display_quit(struct game *cur_game)
{	
	/* Destroy renderer and window */
	SDL_DestroyRenderer(cur_game->screen.renderer);
	cur_game->screen.renderer = NULL;
	SDL_DestroyWindow(cur_game->screen.window);
	cur_game->screen.window = NULL;
	/* Unload sprites and font */
	unload_sprites(cur_game);
	unload_font(cur_game);
	/* SDL quit */
	SDL_Quit();
}

static void
draw_point(struct game *cur_game, int x, int y, char *col)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, *(col+0), *(col+1), *(col+2), 255);
	SDL_RenderDrawPoint(cur_game->screen.renderer, x, y);
}

static void
draw_line(struct game *cur_game, int x1, int y1, int x2, int y2, char *col)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, *(col+0), *(col+1), *(col+2), 255);
	SDL_RenderDrawLine(cur_game->screen.renderer, x1, y1, x2, y2);
}

void
render_clear(struct game *cur_game)
{
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
}

void
render_present(struct game *cur_game)
{
	SDL_RenderPresent(cur_game->screen.renderer);
}

void
draw_rect(struct game *cur_game, unsigned int x, unsigned int y, unsigned int w, unsigned int h, SDL_bool fill, char *fill_col, SDL_bool border, char *bord_col)
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

void
draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index)
{
	SDL_Rect rect = {x, y, w, h};
	SDL_Texture* texture = SDL_CreateTextureFromSurface(cur_game->screen.renderer, cur_game->sprites[sprite_index]);
	SDL_RenderCopy(cur_game->screen.renderer, texture, NULL, &rect);
	SDL_DestroyTexture(texture);
}

void
draw_all(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };

	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	render_present(cur_game);
}

void
draw_game(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int rows, cols;
	short int sprite_index;
	struct win_pos win;
	
	/* Update window position */
	win = find_win_pos(map, cur_player);
	
	/* draw map */
	render_clear(cur_game);
	for (rows = win.y; rows < win.y+WIN_ROWS; rows++) {
		for (cols = win.x; cols < win.x+WIN_COLS; cols++) {
			sprite_index = get_sprite(*(*(map->tile+rows)+cols),
						  *(*(map->biome+rows)+cols));
			draw_tile(cur_game, (cols - win.x) * SPRITE_W + GAME_X, (rows - win.y) * SPRITE_H + GAME_Y, SPRITE_W, SPRITE_H, sprite_index); 
			/* check if there's loot */
			if (*(*(map->loot+rows)+cols) != 0) {
				sprite_index = get_loot_sprite(*(*(map->loot+rows)+cols));
				draw_tile(cur_game, (cols - win.x) * SPRITE_W + GAME_X, (rows - win.y) * SPRITE_H + GAME_Y, SPRITE_W, SPRITE_H, sprite_index); 
			}
		}
	}
	draw_player(cur_game, cur_player, win);
	draw_inv(cur_game, cur_player);
}

static void
draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win)
{	
	draw_tile(cur_game,
		  cur_player->x * SPRITE_W - win.x * SPRITE_W + GAME_X,
		  cur_player->y * SPRITE_H - win.y * SPRITE_H + GAME_Y,
		  SPRITE_W, SPRITE_H, 334);
	cur_player->winpos_x = (cur_player->x * SPRITE_W - win.x * SPRITE_W)/32;
	cur_player->winpos_y = (cur_player->y * SPRITE_H - win.y * SPRITE_H)/32;
}

static void
draw_inv(struct game *cur_game, struct player *cur_player)
{
	int i, j, k;
	short int sprite_index;
	char stackable;
	char darkred[3] = { 128, 0, 0 };
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	char quantity[4];

	/* Draw quick bar */
	draw_rect(cur_game, QB_X, QB_Y, QB_W + 1, QB_H, SDL_TRUE, black, SDL_TRUE, white);
	for (i = 0; i < 8; i++) {
		draw_line(cur_game,
			  QB_X + i*48,
			  QB_Y,
			  QB_X + i*48,
			  QB_Y + 60,
			  white);
		if (cur_player->loot[i] != 0) {
			sprite_index = get_loot_sprite(cur_player->loot[i]);
			draw_tile(cur_game,
				  QB_X + i*48 + 1,
				  QB_Y + 2,
				  SPRITE_W * 1.5, SPRITE_H * 1.5,
				  sprite_index);
			stackable = is_loot_stackable(cur_player->loot[i]);
			if (stackable == STACKABLE && cur_player->quantity[i] > 1) {
				sprintf(quantity, "%3d", cur_player->quantity[i]);
				draw_small_sentence(cur_game,
						    QB_X + i*48 + 1,
						    QB_Y + 48,
						    quantity);
			}
		}
	}
	/* Draw cursor */
	if (cur_game->cursor <= 7) {
		for (i = 0; i < 3; i++) {
			draw_rect(cur_game, WIN_W/2 - (48*8)/2 - i - 1 + 48 * cur_game->cursor, WIN_H - GAME_Y - 16 - 60 - i - 1, 48+3+i*2, 60+2+i*2, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}

	/* Draw inventory? */
	if (cur_game->inventory == SDL_FALSE) return;
	/* Draw inventory rectangle */
	draw_rect(cur_game, INV_X, INV_Y, INV_W, INV_H, SDL_TRUE, black, SDL_TRUE, white);
	/* Draw "Items" text box */
	draw_rect(cur_game, INV_X, INV_Y - 20, INV_W, 18 + 3, SDL_TRUE, black, SDL_TRUE, white);
	draw_small_sentence(cur_game, INV_X + 2, INV_Y - 17, "Inventory");
	/* Draw grid */
	for (i = 0; i < 8; i++) {
		draw_line(cur_game,
			  INV_X, INV_Y + 60 * i,
			  INV_X + INV_W, INV_Y + 60 * i, white);
	}
	for (i = 1; i < 4; i++) {
		draw_line(cur_game,
			  INV_X + i * 48, INV_Y,
			  INV_X + i * 48, INV_Y + INV_H, white);
	}
	
	/* Draw items */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 8; j++) {
			if (cur_player->loot[j+i*8+8] != 0) {
				sprite_index = get_loot_sprite(cur_player->loot[j+i*8+8]);
				draw_tile(cur_game,
					  INV_X + 48 * i + 1,
					  INV_Y + 60 * j + 2, 
					  SPRITE_W * 1.5, SPRITE_H * 1.5,
					  sprite_index);
				stackable = is_loot_stackable(cur_player->loot[j+i*8+8]);
				if (stackable == STACKABLE && cur_player->quantity[j+i*8+8] > 1) {
					sprintf(quantity, "%3d", cur_player->quantity[j+i*8+8]);
					draw_small_sentence(cur_game,
							    INV_X + 48 * i + 1,
							    INV_Y + 60 * j + 48,
							    quantity);
				}
			}
		}
	}
	
	/* Draw cursor */
	if (cur_game->cursor > 7) {
		/* Determine horizontal position */
		if (cur_game->cursor >= 8 && cur_game->cursor < 16) {
			j = INV_X;
		} else if (cur_game->cursor >= 16 && cur_game->cursor < 24) {
			j = INV_X + 48;
		} else if (cur_game->cursor >= 24 && cur_game->cursor < 32) {
			j = INV_X + 96;
		} else if (cur_game->cursor >= 32 && cur_game->cursor < 40) {
			j = INV_X + 144;
		} else {
			return;
		}
		/* Determine vertical position */
		i = INV_Y + 60 * (cur_game->cursor % 8);
		/* Draw cursor */
		for (k = 0; k < 3; k++) {
			draw_rect(cur_game, j - k - 1, i - k - 1, 48+3+k*2, 60+2+k*2 + 1, SDL_FALSE, darkred, SDL_FALSE, NULL);
		}
	}
}

static void
draw_map(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int rows, cols;
	char *tile_col;
	char red[3] = { 255, 0, 0 };
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	
	/* Draw map */
	draw_rect(cur_game, MAP_X, MAP_Y, MAP_W, MAP_H, SDL_TRUE, black, SDL_TRUE, white);
	for (rows = 0; rows < MAP_ROWS; rows++) {
		for (cols = 0; cols < MAP_COLS; cols++) {
			if (*(*(cur_player->seen+rows)+cols) == 0) {
				continue;
			}
			tile_col = get_color(*(*(map->tile + rows) + cols),
					     *(*(map->biome + rows) + cols));
			//draw_rect(cur_game, MAP_X + cols * 2 + 1, MAP_Y + rows * 2 + 1, 2, 2, SDL_TRUE, tile_col, SDL_FALSE, NULL);
			draw_point(cur_game, MAP_X + cols, MAP_Y + rows, tile_col);
		}
	}
	/* Draw player */
	draw_point(cur_game, MAP_X + cur_player->x, MAP_Y + cur_player->y, red);
	draw_rect(cur_game, MAP_X, MAP_Y, MAP_W, MAP_H, SDL_FALSE, white, SDL_FALSE, NULL);

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
	} else if (win.x > map->col_size - WIN_COLS) {
		win.x = map->col_size - WIN_COLS;
	}
	
	/* correct y value */
	if (win.y < 0) {
		win.y = 0;
	} else if (win.y > map->row_size - WIN_ROWS) {
		win.y = map->row_size - WIN_ROWS;
	}
	
	return win;
}

void
worldmap(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char black[3] = { 0, 0, 0 };
	char white[3] = { 255, 255, 255 };
	
	/* Redraw screen */
	draw_game(cur_game, map, cur_player);
	/* Draw map */
	draw_map(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	/* Write "Map" at the top of the screen */
	draw_rect(cur_game, MAP_X, MAP_Y-20, 28*9+2, 18+2+1, SDL_TRUE, black, SDL_TRUE, white);
	draw_sentence(cur_game, MAP_X+1, MAP_Y-19, "World Map");
	/* Render */
	SDL_RenderPresent(cur_game->screen.renderer);
	/* Wait for user input */
	SDL_Event event;
	while (SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			return;
		}
	}
}

static void
load_sprites(struct game *cur_game)
{
	/* Load the map arrow sprites */
	SDL_Surface *surface = SDL_LoadBMP("art/sprites.bmp");
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

static void
unload_sprites(struct game *cur_game)
{
	int i;

	/* Free all sprites */
	for (i = 0; i < 512; i++) {
		SDL_FreeSurface(cur_game->sprites[i]);
	}
	free(cur_game->sprites);
}

int LAST = 0;
void
loading_bar(struct game *cur_game, char *title, int percentage)
{
	char black[3] = { 0, 0, 0 };
	char white[3] = { 255, 255, 255 };
	char blue[3] = { 0, 0, 255 };
	
	/* Should I even draw the loading bar? */
	if (percentage == LAST) return;
	/* Yes */
	LAST = percentage;
	
	/* Clear screen */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	/* Write title to screen */
	draw_sentence(cur_game, 0, 0, title);
	/* Draw percentage bar */
	draw_rect(cur_game, 20, 30, 1000, 40, SDL_TRUE, black, SDL_TRUE, white);
	draw_rect(cur_game, 21, 31, percentage*10-2, 38, SDL_TRUE, blue, SDL_FALSE, NULL);
	SDL_RenderPresent(cur_game->screen.renderer);
}
