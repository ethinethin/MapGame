#define DEVMODE

#include <stdio.h>
#include <SDL2/SDL.h>
#include "devm.h"
#include "disp.h"
#include "harv.h"
#include "hold.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "maus.h"
#include "play.h"
#include "rand.h"

/* Function prototypes */
static void	game_init(void);
static void	game_quit(void);
static void	generate_farts(struct game *cur_game, struct worldmap *main_map);
static void	copy_fart(struct worldmap *main_map, struct worldmap *fart, int row, int col);

/* Global game construct */
struct game GAME = {
	SDL_FALSE,		/* running */
	/* screen */
	{ WIN_W, WIN_H, "MapGame", NULL, NULL },
	NULL,			/* font */
	NULL,			/* sprite_textures */
	NULL,			/* map_texture */
	0,			/* cursor */
	SDL_FALSE,		/* inventory */
	SDL_FALSE		/* fullscreen */
};

/* Global player construct */
struct player PLAYER;

/* Global map construct */
struct worldmap MAP;

int
main()
{
	/* initialize game */
	game_init();
	
	/* draw map, player, render and "move player" to update map */
	draw_all(&GAME, &MAP, &PLAYER);
	move_player(&GAME, &MAP, &PLAYER, 0, 0);
	
	/* Reveal the map */
	#ifdef DEVMODE
	reveal_map(&GAME, &MAP, &PLAYER);
	give_me_floors(&PLAYER);
	#endif
	
	/* enter main game loop */
	SDL_Event event;
	while (GAME.running) {
		/* draw map, player, and render */
		draw_all(&GAME, &MAP, &PLAYER);
		SDL_Delay(10);
		/* poll for an event */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_QUIT) { /* exit button pressed */
			GAME.running = SDL_FALSE;
		} else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: /* quit */
					GAME.running = SDL_FALSE;
					break;
				case SDLK_UP: /* move up */
				case SDLK_w:
					move_player(&GAME, &MAP, &PLAYER, 0, -1);
					break;
				case SDLK_DOWN: /* move down */
				case SDLK_s:
					move_player(&GAME, &MAP, &PLAYER, 0, 1);
					break;
				case SDLK_LEFT: /* move left */
				case SDLK_a:
					move_player(&GAME, &MAP, &PLAYER, -1, 0);
					break;
				case SDLK_RIGHT: /* move right */
				case SDLK_d:
					move_player(&GAME, &MAP, &PLAYER, 1, 0);
					break;
				case SDLK_m: /* view map */
					worldmap(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_i: /* toggle inventory */
					toggle_inv(&GAME);
					break;
				case SDLK_q: /* move cursor left */
					move_cursor(&GAME, -1);
					break;
				case SDLK_e: /* move cursor right */
					move_cursor(&GAME, 1);
					break;
				case SDLK_t:	/* throw */
					throw_item(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_r:	/* retrieve */
					pickup_item(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_f: /* swap item */
					swap_item(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_z: /*harvest item */
					get_harvest_input(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_1: /* move to quickbar slot 1 */
					move_cursor(&GAME, 0 - GAME.cursor);
					break;
				case SDLK_2: /* move to quickbar slot 2 */
					move_cursor(&GAME, 1 - GAME.cursor);
					break;
				case SDLK_3: /* move to quickbar slot 3 */
					move_cursor(&GAME, 2 - GAME.cursor);
					break;
				case SDLK_4: /* move to quickbar slot 4 */
					move_cursor(&GAME, 3 - GAME.cursor);
					break;
				case SDLK_5: /* move to quickbar slot 5 */
					move_cursor(&GAME, 4 - GAME.cursor);
					break;
				case SDLK_6: /* move to quickbar slot 6 */
					move_cursor(&GAME, 5 - GAME.cursor);
					break;
				case SDLK_7: /* move to quickbar slot 7 */
					move_cursor(&GAME, 6 - GAME.cursor);
					break;
				case SDLK_8: /* move to quickbar slot 8 */
					move_cursor(&GAME, 7 - GAME.cursor);
					break;
				default:
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			mouse_click(&GAME, &MAP, &PLAYER, event.button.x, event.button.y, event.button.button);
		}
	}
	/* quit game and exit normally */
	game_quit();
	return 0;
}

static void
game_init(void)
{
	/* Initialize the game */
	display_init(&GAME);
	/* Seed RNG */
	seed_rng();
	/* Set up the worldmap */
	create_map(&MAP, MAP_ROWS, MAP_COLS);
	/* Generate farts and copy to worldmap */
	generate_farts(&GAME, &MAP);
	/* Set up player */
	player_init(&MAP, &PLAYER);
	/* Set up depleted item table */
	setup_dtable();
	/* Set up holder table */
	setup_hold();
	/* The window is now up and running */
	GAME.running = SDL_TRUE;
}

static void
game_quit(void)
{
	/* Delete holder table */
	kill_hold();
	/* Delete depleted item table */
	kill_dtable();
	/* Free the player */
	player_quit(&PLAYER);
	/* Free the world map and exit normally */
	free_map(&MAP);
	/* Quit SDL display functions */
	display_quit(&GAME);	
}

static void
generate_farts(struct game *cur_game, struct worldmap *main_map)
{
	int rows, cols;
	int rando;
	struct worldmap biomes;
	struct worldmap fart;

	/* Pick random biomes */
	create_map(&biomes, 32, 64);
	populate_map(&biomes, 1, 5);
	
	/* Create farts */
	for (rows = 0; rows < 32; rows++) {
		/* Loading bar */
		loading_bar(cur_game, "Generating maps", 100*rows/32);
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
	
	// Add a bunch of random items
	// Remove later
	for (rows = 0; rows < main_map->row_size - 1; rows++) {
		for (cols = 0; cols < main_map->col_size - 1; cols++) {
			rando = rand_num(1, 100);
			if (rando == 100 && is_passable(*(*(main_map->tile+rows)+cols),
						     *(*(main_map->biome+rows)+cols)) == PASSABLE) {
				rando = rand_num(1, 8);
				*(*(main_map->loot+rows)+cols) = rando;
				if (is_loot_stackable(rando) == STACKABLE) {
					*(*(main_map->quantity+rows)+cols) = rand_num(30,50);
				} else {
					*(*(main_map->quantity+rows)+cols) = 1;
				}
			}
		}
	}
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
