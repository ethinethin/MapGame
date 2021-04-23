//#define DEVMODE

#include <stdio.h>
#include <SDL2/SDL.h>
#include "devm.h"
#include "disp.h"
#include "harv.h"
#include "hold.h"
#include "home.h"
#include "loot.h"
#include "main.h"
#include "make.h"
#include "maps.h"
#include "maus.h"
#include "play.h"
#include "rand.h"
#include "save.h"

/* Function prototypes */
static void	game_init(void);
static void	game_quit(void);

/* Global game construct */
struct game GAME = {
	SDL_FALSE,		/* running */
	/* screen */
	{ WIN_W, WIN_H, "MapGame", NULL, NULL },
	NULL,			/* font */
	NULL,			/* sprite_textures */
	NULL,			/* map_texture */
	NULL,			/* scanlines texture */
	NULL,			/* craft texture */
	0,			/* cursor */
	SDL_FALSE,		/* inventory */
	SDL_FALSE,		/* fullscreen */
	SDL_TRUE		/* scanlines enabled */
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
	
	/* go to the title screen */
	title_screen(&GAME, &MAP, &PLAYER, STARTING_GAME);
	if (GAME.running == SDL_FALSE) return 0;
	
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
					title_screen(&GAME, &MAP, &PLAYER, GAME_IN_PROGRESS);
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
				case SDLK_z: /* harvest item */
					get_harvest_input(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_c: /* craft item */
					make_stuff(&GAME, &MAP, &PLAYER);
					break;
				case SDLK_l: /* toggle scanLines */
					toggle_scanlines(&GAME);
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
