#include <stdio.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "main.h"
#include "maps.h"
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
	NULL,			/* sprites */
	NULL,			/* font */
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
	/* draw map, player, and render */
	draw_all(&GAME, &MAP, &PLAYER);
	/* enter main game loop */
	SDL_Event event;
	while(GAME.running && SDL_WaitEvent(&event)) {
		if (event.type == SDL_QUIT) {
			GAME.running = SDL_FALSE;
		} else if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_ESCAPE:
					GAME.running = SDL_FALSE;
					break;
				case SDLK_UP:
				case SDLK_w:
					move_player(&MAP, &PLAYER, 0, -1);
					break;
				case SDLK_DOWN:
				case SDLK_s:
					move_player(&MAP, &PLAYER, 0, 1);
					break;
				case SDLK_LEFT:
				case SDLK_a:
					move_player(&MAP, &PLAYER, -1, 0);
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					move_player(&MAP, &PLAYER, 1, 0);
					break;
				case SDLK_m:
					worldmap(&GAME, &MAP, &PLAYER);
				default:
					break;
			}
		}
		/* draw map, player, and render */
		draw_all(&GAME, &MAP, &PLAYER);
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
	/* Set up player */
	player_init(&PLAYER);
	/* Set up the worldmap */
	create_map(&MAP, MAP_ROWS, MAP_COLS);
	/* Generate farts and copy to worldmap */
	generate_farts(&GAME, &MAP);
	/* The window is now up and running */
	GAME.running = SDL_TRUE;
}

static void
game_quit(void)
{
	/* Quit SDL display functions */
	display_quit(&GAME);
	/* Free the world map and exit normally */
	free_map(&MAP);
	/* Free the player */
	player_quit(&PLAYER);
}

static void
generate_farts(struct game *cur_game, struct worldmap *main_map)
{
	int rows, cols;
	struct worldmap biomes;
	struct worldmap fart;

	/* Pick random biomes */
	create_map(&biomes, 32, 64);
	populate_map(&biomes, 3, 5);
	
	/* Create 4 fart */
	for (rows = 0; rows < 32; rows++) {
		/* Loading bar */
		loading_bar(cur_game, "Generating maps", 100*rows/32);
		for (cols = 0; cols < 64; cols++) {
			/* Create a fart */
			create_map(&fart, 16, 16);
			/* Populate with a random biome */
			populate_map(&fart, 1, *(*(biomes.tile+rows)+cols) - 1);
			/* copy_fart */
			copy_fart(main_map, &fart, rows * 16, cols * 16);
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
	
	for (rows = 0; rows < fart->row_size; rows++) {
		for (cols = 0; cols < fart->col_size; cols++) {
			*(*(main_map->tile+rows+row)+cols+col) = *(*(fart->tile+rows)+cols);
			*(*(main_map->biome+rows+row)+cols+col) = *(*(fart->biome+rows)+cols);
		}
	}
}
