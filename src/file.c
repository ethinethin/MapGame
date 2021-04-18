#include <SDL2/SDL.h>
#include "disp.h"
#include "file.h"
#include "font.h"
#include "harv.h"
#include "hold.h"
#include "main.h"
#include "maps.h"
#include "play.h"
#include "rand.h"
#include "save.h"

//#define START

/* Function prototypes */
static void	generate_farts(struct game *cur_game, struct worldmap *main_map);
static void	copy_fart(struct worldmap *main_map, struct worldmap *fart, int row, int col);
static void	draw_title_screen(struct game *cur_game, int status);
static void	setup_tiles(void);

void
title_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player, int status)
{
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	int new_game;
	setup_tiles();
	render_clear(cur_game);
	while(finished == SDL_FALSE) {
		/* Redraw screen */
		render_clear(cur_game);
		draw_title_screen(cur_game, status);
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_n: /* NEW GAME */
					if (status == STARTING_GAME) {
						new_game = STARTING_GAME;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_s: /* SAVE GAME */
					if (status == GAME_IN_PROGRESS) {
						new_game = GAME_IN_PROGRESS;
						finished = SDL_TRUE;
						save_all(map, cur_player);
					}
					break;
				case SDLK_c: /* CONTINUE */
				case SDLK_ESCAPE:
					if (status == STARTING_GAME) {
						new_game = CONTINUE_GAME;
					} else {
						new_game = GAME_IN_PROGRESS;
					}
					finished = SDL_TRUE;
					break;
				case SDLK_o: /* OPTIONS */
					break;
				case SDLK_q: /* QUIT */
					finished = SDL_TRUE;
					new_game = QUITTING_GAME;
					break;
			}
		} else if (event.type == SDL_QUIT) {
			new_game = QUITTING_GAME;
			finished = SDL_TRUE;
		}
	}
	if (new_game == STARTING_GAME) {
		/* Set up the worldmap */
		create_map(map, MAP_ROWS, MAP_COLS);
		/* Generate farts and copy to worldmap */
		generate_farts(cur_game, map);
		/* Set up player */
		player_init(map, cur_player);
		/* Set up depleted item table */
		setup_dtable();
		/* Set up holder table */
		setup_hold();
	} else if (new_game == CONTINUE_GAME) {
		load_all(cur_game, map, cur_player);
	} else if (new_game == QUITTING_GAME) {
		cur_game->running = SDL_FALSE;
	}

}

static void
generate_farts(struct game *cur_game, struct worldmap *main_map)
{
	int rows, cols;
	struct worldmap biomes;
	struct worldmap fart;

	/* Pick random biomes */
	create_map(&biomes, 32, 64);
	populate_map(&biomes, 1, 5);
	
	/* Create farts */
	for (rows = 0; rows < 32; rows++) {
		/* Loading bar */
		loading_bar(cur_game, "GENERATING MAPS", 100*rows/32);
		for (cols = 0; cols < 64; cols++) {
			/* Create a fart */
			create_map(&fart, 24, 24);
			/* Populate with a random biome */
			populate_map(&fart, 1, *(*(biomes.tile+rows)+cols) - 1);
			/* copy_fart */
			copy_fart(main_map, &fart, rows * 16 - 4, cols * 16 - 4);
			/* free_map */
			free_map(&fart);
		}
	}
	/* Free the biome plan */
	free_map(&biomes);
}

static void
copy_fart(struct worldmap *main_map, struct worldmap *fart, int row, int col)
{
	int rows, cols;
	
	/* Make sure it fits in map */
	if (row < 0) {
		fart->row_size += row;
		row = 0;
	}
	if (col < 0) {
		fart->col_size += col;
		col = 0;
	}
	
	for (rows = 0; rows < fart->row_size; rows++) {
		for (cols = 0; cols < fart->col_size; cols++) {
			/* Make sure you're within the map */
			if (rows + row > main_map->row_size - 1) continue;
			if (cols + col > main_map->col_size - 1) continue;
			/* If there's an existing tile, blend it maybe */
			if (*(*(main_map->tile+rows+row)+cols+col) == 0 || rand_num(0, 1) == 1) {
				*(*(main_map->tile+rows+row)+cols+col) = *(*(fart->tile+rows)+cols);
				*(*(main_map->biome+rows+row)+cols+col) = *(*(fart->biome+rows)+cols);
			}
		}
	}
}

int tile[80][45];
static void
setup_tiles(void)
{
	int i, j;
	// setup random tiles
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 45; j++) {
			tile[i][j] = rand_num(512, 559);
		}
	}
}


static void
draw_title_screen(struct game *cur_game, int status)
{
	int i, j;
	
	// change one random tile
	tile[(short int) rand_num(0, 79)][(short int) rand_num(0, 44)] = rand_num(512, 559);
	
	// draw some random tiles
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 45; j++) {
			draw_tile(cur_game, i*16, j*16, 16, 16, tile[i][j], 128);
		}
	}

	// draw game title
	draw_sentence(cur_game, 100, 100, "MAPGAME");
	// draw new game
	if (status == STARTING_GAME) {
		draw_small_sentence(cur_game, 120, 150, "NEW GAME");
	} else {
		draw_small_sentence(cur_game, 120, 150, "SAVE GAME");
	}
	// draw continue
	draw_small_sentence(cur_game, 120, 170, "CONTINUE");
	draw_small_sentence(cur_game, 120, 190, "OPTIONS");
	draw_small_sentence(cur_game, 120, 210, "EXIT");
}
