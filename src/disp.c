#include <stdio.h>
#include <string.h>
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
static void		draw_point(struct game *cur_game, int x, int y, char *col);
static void		draw_rect(struct game *cur_game, unsigned int x, unsigned int y,
				  unsigned int w, unsigned int h, SDL_bool fill,
			  	  char fill_col[3], SDL_bool border, char bord_col[3]);
static void		draw_tile(struct game *cur_game, int x, int y, int w, int h, int sprite_index);
static void		draw_game(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static void		draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win);
static void		draw_map(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static struct win_pos	find_win_pos(struct worldmap *map, struct player *cur_player);
static void		update_seen(struct worldmap *map, struct player *cur_player);
static void		draw_char(struct game *cur_game, int x, int y, int letter);
static void		load_sprites(struct game *cur_game);
static void		unload_sprites(struct game *cur_game);
static void		load_font(struct game *cur_game);
static void		unload_font(struct game *cur_game);


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

static void
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
	draw_game(cur_game, map, cur_player);
	SDL_RenderPresent(cur_game->screen.renderer);
}

static void
draw_game(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int rows, cols;
	short int sprite_index;
	char white[3] = { 255, 255, 255 };
	struct win_pos win;
	
	/* Update window position */
	win = find_win_pos(map, cur_player);

	/* Update seen */
	update_seen(map, cur_player);
	
	/* draw map */
	SDL_SetRenderDrawColor(cur_game->screen.renderer, 0, 0, 0, 255);
	SDL_RenderClear(cur_game->screen.renderer);
	for (rows = win.y; rows < win.y+WIN_ROWS; rows++) {
		for (cols = win.x; cols < win.x+WIN_COLS; cols++) {
			sprite_index = get_sprite(*(*(map->tile+rows)+cols),
						  *(*(map->biome+rows)+cols));
			draw_tile(cur_game, (cols - win.x) * SPRITE_W + GAME_X, (rows - win.y) * SPRITE_H + GAME_Y, SPRITE_W, SPRITE_H, sprite_index); 
		}
	}
	draw_player(cur_game, cur_player, win);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
}

static void
draw_player(struct game *cur_game, struct player *cur_player, struct win_pos win)
{	
	draw_tile(cur_game,
		  cur_player->x*SPRITE_W - win.x*SPRITE_W + GAME_X,
		  cur_player->y*SPRITE_H - win.y*SPRITE_H + GAME_Y,
		  SPRITE_W, SPRITE_H, 328); 
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

static void
update_seen(struct worldmap *map, struct player *cur_player)
{
	int rows, cols;
	int rows_i, cols_i, rows_f, cols_f;
	
	rows_i = cur_player->y - 9;
	cols_i = cur_player->x - 19;
	rows_f = cur_player->y + 9;
	cols_f = cur_player->x + 19;
	
	/* Check boundaries for loop */
	if (rows_i < 0) {
		rows_i = 0;
		rows_f = WIN_ROWS - 1;
	}
	if (cols_i < 0) {
		cols_i = 0;
		cols_f = WIN_COLS - 1;
	}
	if (rows_f > map->row_size - 1) {
		rows_f = map->row_size - 1;
		rows_i = rows_f - WIN_ROWS + 1;
	}
	if (cols_f > map->col_size - 1) {
		cols_f = map->col_size - 1;
		cols_i = cols_f - WIN_COLS + 1;
	}
	
	/* Set all seen values to 1 */
	for (rows = rows_i ; rows <= rows_f; rows++) {
		for (cols = cols_i; cols <= cols_f; cols++) {
			*(*(cur_player->seen+rows)+cols) = 1;
		}
	}
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
	/* Write "Map" at the top of the screen */
	draw_rect(cur_game, MAP_X, MAP_Y-20, 28*9+2, 18+2, SDL_TRUE, black, SDL_TRUE, white);
	draw_sentence(cur_game, MAP_X+1, MAP_Y-19, "World Map");
	/* Render */
	SDL_RenderPresent(cur_game->screen.renderer);
	/* Wait for user input */
	SDL_Event event;
	while(SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			return;
		}
	}
}

static void
draw_char(struct game *cur_game, int x, int y, int letter)
{
	SDL_Rect rect = {x, y, 28, 18};
	SDL_Texture* texture = SDL_CreateTextureFromSurface(cur_game->screen.renderer, cur_game->font[letter]);
	SDL_RenderCopyEx(cur_game->screen.renderer, texture, NULL, &rect, 0, NULL, 0);
	SDL_DestroyTexture(texture);
}

void
draw_sentence(struct game *cur_game, int start_x, int start_y, const char *sentence)
{
	int i;
	int len;
	int x = start_x;
	int y = start_y;

	for (i = 0, len = strlen(sentence); i < len; i++) {
		draw_char(cur_game, x, y, sentence[i] - 32);
		x += 28;
		if (x >= 1270) {
			x = start_x;
			y = y + 18;
		}
	}

}

static void
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

static void
load_font(struct game *cur_game)
{
	int i, j;
	int count = 0;
	SDL_Surface* surface;
	SDL_Rect rect = {1, 1, 28, 18};

	/* Allocate memory for 96 font characters */
	cur_game->font = (SDL_Surface**) malloc(sizeof(SDL_Surface*)*96);
	/* Load sprite sheet */
	surface = SDL_LoadBMP("art/font.bmp");
	/* Load all sprites */
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 16; j++) {
			rect.x = i*28 + i + 1;
			rect.y = j*18 + j + 1;
			cur_game->font[count] = SDL_CreateRGBSurface(0, 28, 18, 24, 0, 0, 0, 0);
			SDL_BlitSurface(surface, &rect, cur_game->font[count], NULL);
			count++;
		}
	}
	SDL_FreeSurface(surface);
}

static void
unload_font(struct game *cur_game)
{
	int i;

	/* Free all font characters */
	for (i = 0; i < 96; i++) {
		SDL_FreeSurface(cur_game->font[i]);
	}
	free(cur_game->font);
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
