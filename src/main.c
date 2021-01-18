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

/* Global game construct */
struct game GAME = {
	SDL_FALSE,		/* running */
	/* screen */
	{ WIN_W, WIN_H, "MapGame", NULL, NULL },
	NULL,			/* sprites */
	SDL_FALSE		/* fullscreen */
};

/* Global player construct */
struct player PLAYER;

/* Global map construct */
struct worldmap MAP;

int
main(int argc, char *argv[])
{

	/* initialize game */
	game_init();

	/* draw map, player, and render */
	draw_game(&GAME, &MAP, &PLAYER);

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
					move_player(&GAME, &MAP, &PLAYER, 0, -1);
					break;
				case SDLK_DOWN:
				case SDLK_s:
					move_player(&GAME, &MAP, &PLAYER, 0, 1);
					break;
				case SDLK_LEFT:
				case SDLK_a:
					move_player(&GAME, &MAP, &PLAYER, -1, 0);
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					move_player(&GAME, &MAP, &PLAYER, 1, 0);
					break;
				default:
					break;
			}
		}
		/* draw map, player, and render */
		draw_game(&GAME, &MAP, &PLAYER);
	}

	/* quit game and exit normally */
	game_quit();
	return 0;
}

static void
game_init(void)
{
	/* Seed RNG */
	seed_rng();
	/* Set up player */
	player_init(&PLAYER);
	/* Set up the worldmap */
	create_map(&MAP, MAP_ROWS, MAP_COLS);
	/* Populate the worldmap */
	populate_map(&MAP, 1);
	/* Initialize the game */
	display_init(&GAME);
	/* Load sprites */
	load_sprites(&GAME);
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
