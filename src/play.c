#include <stdio.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"
#include "rand.h"

/* Function prototypes */
static void	update_seen(struct game *cur_game, struct worldmap *map, struct player *cur_player);

void
player_init(struct worldmap *map, struct player *cur_player)
{
	int rows, cols;
	/* Set player position */
	random_start(map, cur_player);
	/* Initialize player seen */
	cur_player->seen = malloc(sizeof(*cur_player->seen)*MAP_ROWS);
	for (rows = 0; rows < MAP_ROWS; rows++) {
		*(cur_player->seen+rows) = malloc(sizeof(**cur_player->seen)*MAP_COLS);
		for (cols = 0; cols < MAP_COLS; cols++) {
			*(*(cur_player->seen+rows)+cols) = 0;
		}
	}
	/* Initialize the inventory */
	for (rows = 0; rows < MAX_INV; rows++) {
		cur_player->loot[rows] = 0;
		cur_player->quantity[rows] = 0;
	}
}

void
player_quit(struct player *cur_player)
{
	int rows;
	
	/* Free memory allocated for the player */
	for (rows = 0; rows < MAP_ROWS; rows++) {
		free(*(cur_player->seen+rows));
	}
	free(cur_player->seen);
}

void
move_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	int new_x, new_y;
	
	/* Determine new coordinates */
	new_x = cur_player->x + x;
	new_y = cur_player->y + y;
	/* Pac-man the player */
	if (new_x < 0) new_x = map->col_size - 1; else if (new_x > map->col_size - 1) new_x = 0;
	if (new_y < 0) new_y = map->row_size - 1; else if (new_y > map->row_size - 1) new_y = 0;	
	/* check if tile is impassable */
	if (is_passable(*(*(map->tile+new_y)+new_x),
	                *(*(map->biome+new_y)+new_x)) == IMPASSABLE ||
	    is_loot_passable(*(*(map->loot+new_y)+new_x)) == IMPASSABLE) {
	    	return;
	}
	/* move player */
	cur_player->x = new_x;
	cur_player->y = new_y;
	/* Update seen */
	update_seen(cur_game, map, cur_player);
}

static void
update_seen(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char *tile_col;
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

	/* Set up renderer to render to the map texture */
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->map_texture);
	/* Set all newly seen values to 1 */
	for (rows = rows_i ; rows <= rows_f; rows++) {
		for (cols = cols_i; cols <= cols_f; cols++) {
			if (*(*(cur_player->seen+rows)+cols) == 0) {
				/* Set tile as seen */
				*(*(cur_player->seen+rows)+cols) = 1;
				/* Draw pixel on map texture */
				tile_col = get_color(*(*(map->tile + rows) + cols), *(*(map->biome + rows) + cols));
				draw_point(cur_game, cols, rows, tile_col);
			}
		}
	}
	/* Reset renderer */
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
}

void
toggle_inv(struct game *cur_game)
{
	cur_game->inventory = !cur_game->inventory;
}

void
random_start(struct worldmap *map, struct player *cur_player)
{
	int row, col;
	SDL_bool finished = SDL_FALSE;
	
	while (finished == SDL_FALSE) {
		/* Randomly determine location */
		row = rand_num(0, map->row_size - 1);
		col = rand_num(0, map->col_size - 1);
		/* Check that it is passable */
		if (is_passable(*(*(map->tile+row)+col), *(*(map->biome+row)+col)) == PASSABLE) {			
			finished = SDL_TRUE;
		}
	}
	cur_player->x = col;
	cur_player->y = row;
}
