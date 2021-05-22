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
#include "npcs.h"
#include "play.h"
#include "rand.h"
#include "save.h"

/* Function prototypes */
static void	game_init(void);
static void	game_quit(void);
static void	display_reloaded(struct game *cur_game, struct worldmap *map, struct player *cur_player);

/* Global game construct */
struct game GAME = {
	SDL_FALSE,		/* running */
	/* screen */
		{
		  WIN_W,		/* window width */
		  WIN_H,		/* window height */
		  1.0,			/* scale x */
		  1.0,			/* scale y */
		  SDL_FALSE,		/* vsync */
		  2,			/* displaymode */
		  SDL_TRUE,		/* scanlines_on */
		  SDL_FALSE,		/* display_reloaded */
		  "MapGame",		/* window name */
		  NULL,			/* window */
		  NULL,			/* renderer */
		  NULL			/* output */
		},
	NULL,			/* font */
	NULL,			/* sprite_textures */
	NULL,			/* map_texture */
	NULL,			/* scanlines texture */
	NULL,			/* craft texture */
	0,			/* cursor */
	SDL_FALSE,		/* inventory */
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
		/* check if display was reloaded and act accordingly */
		if (GAME.screen.display_reloaded == SDL_TRUE) {
			display_reloaded(&GAME, &MAP, &PLAYER);
			GAME.screen.display_reloaded = SDL_FALSE;
		}
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
			mouse_click(&GAME, &MAP, &PLAYER, event.button.x / GAME.screen.scale_x, event.button.y / GAME.screen.scale_y, event.button.button);
		}
	}
	/* quit game and exit normally */
	game_quit();
	return 0;
}

static void
game_init(void)
{
	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL error: %s\n", SDL_GetError());
		exit(1);
	}
	/* Load display options */
	load_opts(&GAME);
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
	/* Delete NPC table */
	kill_npcs();
	/* Free the player */
	player_quit(&PLAYER);
	/* Free the world map and exit normally */
	free_map(&MAP);
	/* Quit SDL display functions */
	display_quit(&GAME);
	/* SDL quit */
	SDL_Quit();
}

static void
display_reloaded(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char *tile_col;
	int i, j;
	#ifdef LOADBAR
	int perc = 0;
	#endif
	
	/* Reloaded map texture */
	for (i = 0, j = 0; i < map->row_size; i++) {
		#ifdef LOADBAR
		perc = 100 * ((i*map->col_size)+j) / (map->col_size * map->row_size);
		loading_bar(cur_game, "Reloading map texture", perc);
		#endif
		for (j = 0 ; j < map->col_size; j++) {
			if (*(*(cur_player->seen+i)+j) == 1) {
				SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->map_texture);
				tile_col = get_color(*(*(map->tile+i)+j), *(*(map->biome+i)+j));
				draw_point(cur_game, j, i, tile_col);
			}
		}
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->screen.output);
}
