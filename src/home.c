#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "harv.h"
#include "hold.h"
#include "home.h"
#include "main.h"
#include "maps.h"
#include "npcs.h"
#include "play.h"
#include "rand.h"
#include "save.h"

//#define START

/* Function prototypes */
static void	generate_farts(struct game *cur_game, struct worldmap *main_map);
static void	copy_fart(struct worldmap *main_map, struct worldmap *fart, int row, int col);
static void	setup_tiles(struct game *cur_game);
static void	draw_title_tiles(struct game *cur_game);
static void	draw_title_tile(struct game *cur_game, int i, int j);
static void	draw_title_screen(struct game *cur_game, int status);
static void	draw_bg(struct game *cur_game);
static void	save_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static SDL_bool	load_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static void	opts_screen(struct game *cur_game);
static void	draw_saveload_screen(struct game *cur_game, SDL_bool *saves, SDL_bool save);
static void	draw_opts_screen(struct game *cur_game);
static void	draw_yesno_screen(struct game *cur_game, char *message);


void
title_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player, int status)
{
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	int new_game;
	int x, y;
	
	/* Setup random tiles and enter input loop */
	setup_tiles(cur_game);
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
						save_screen(cur_game, map, cur_player);
					}
					break;
				case SDLK_c: /* CONTINUE */
				case SDLK_l: /* LOAD GAME */
				case SDLK_ESCAPE:
					if (status == STARTING_GAME) {
						new_game = CONTINUE_GAME;
						finished = load_screen(cur_game, map, cur_player);
					} else {
						new_game = GAME_IN_PROGRESS;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_o: /* OPTIONS */
					opts_screen(cur_game);
					break;
				case SDLK_q: /* QUIT */
				case SDLK_x: /* EXIT */
					finished = SDL_TRUE;
					new_game = QUITTING_GAME;
					break;
			}
		} else if (event.type == SDL_QUIT) {
			new_game = QUITTING_GAME;
			finished = SDL_TRUE;
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			x = event.button.x; y = event.button.y;
			if (x >= 123  * cur_game->screen.scale_x && x <= 219  * cur_game->screen.scale_x &&
			    y >= 153 * cur_game->screen.scale_y && y <= 162 * cur_game->screen.scale_y) {
				/* new game or save game */
				if (status == STARTING_GAME) {
					new_game = STARTING_GAME;
					finished = SDL_TRUE;
				} else if (status == GAME_IN_PROGRESS) {
					new_game = GAME_IN_PROGRESS;
					finished = SDL_TRUE;
					save_screen(cur_game, map, cur_player);
				}
			} else if (x >= 123 * cur_game->screen.scale_x && x <= 219 * cur_game->screen.scale_x &&
				   y >= 175 * cur_game->screen.scale_y && y <= 183 * cur_game->screen.scale_y) {
				/* continue */
				if (status == STARTING_GAME) {
					new_game = CONTINUE_GAME;
					finished = load_screen(cur_game, map, cur_player);
				} else {
					new_game = GAME_IN_PROGRESS;
					finished = SDL_TRUE;
				}
			} else if (x >= 123 * cur_game->screen.scale_x && x <= 195 * cur_game->screen.scale_x &&
				   y >= 193 * cur_game->screen.scale_y && y <= 203 * cur_game->screen.scale_y) {
				/* options */
				opts_screen(cur_game);
				continue;
			} else if (x >= 123 * cur_game->screen.scale_x && x <= 161 * cur_game->screen.scale_x &&
				   y >= 213 * cur_game->screen.scale_y && y <= 222 * cur_game->screen.scale_y) {
				/* exit */
				finished = SDL_TRUE;
				new_game = QUITTING_GAME;
			}
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
		/* Set up NPCs */
		setup_npcs();
		populate_npcs(map);
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

SDL_Texture *title_screen_tex = NULL;
int tile[80][45];
static void
setup_tiles(struct game *cur_game)
{
	int i, j;
	/* Setup random tiles for title screen */
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 45; j++) {
			tile[i][j] = rand_num(512, 559);
		}
	}
	/* Create texture if necessary */
	if (title_screen_tex == NULL) {
		title_screen_tex = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
						 SDL_TEXTUREACCESS_TARGET, cur_game->screen.w, cur_game->screen.h);
	}
	/* Output all tiles to texture */
	draw_title_tiles(cur_game);

}

static void
draw_title_tiles(struct game *cur_game)
{
	char black[3] = { 0, 0, 0 };
	int i, j;
	
	/* Render to texture */
	SDL_SetRenderTarget(cur_game->screen.renderer, title_screen_tex);
	SDL_SetTextureBlendMode(title_screen_tex, SDL_BLENDMODE_BLEND);
	draw_rect(cur_game, 0, 0, cur_game->screen.w, cur_game->screen.h, SDL_TRUE, black, SDL_FALSE, NULL);
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 45; j++) {
			draw_tile(cur_game, i * 16 * cur_game->screen.scale_x, j * 16 * cur_game->screen.scale_y,
				  16 * cur_game->screen.scale_x, 16 * cur_game->screen.scale_y, tile[i][j], 128);
		}
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
}

static void
draw_title_tile(struct game *cur_game, int i, int j)
{
	char black[3] = { 0, 0, 0 };
	
	/* Render to texture */
	SDL_SetRenderTarget(cur_game->screen.renderer, title_screen_tex);
	SDL_SetTextureBlendMode(title_screen_tex, SDL_BLENDMODE_BLEND);
	draw_rect(cur_game, i * 16 * cur_game->screen.scale_x, j * 16 * cur_game->screen.scale_y, 16 * cur_game->screen.scale_x, 16 * cur_game->screen.scale_y, SDL_TRUE, black, SDL_FALSE, NULL);
	draw_tile(cur_game, i * 16 * cur_game->screen.scale_x, j * 16 * cur_game->screen.scale_y, 16 * cur_game->screen.scale_x, 16 * cur_game->screen.scale_y, tile[i][j], 128);
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
}

static void
draw_title_screen(struct game *cur_game, int status)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* draw game title */
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 100 * cur_game->screen.scale_y, "MAPGAME");
	/* draw new game */
	if (status == STARTING_GAME) {
		draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "NEW GAME");
	} else {
		draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "SAVE GAME");
	}
	/* draw continue */
	if (status == STARTING_GAME) {
		draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 170 * cur_game->screen.scale_y, "LOAD GAME");
	} else {
		draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 170 * cur_game->screen.scale_y, "CONTINUE");
	}
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 190 * cur_game->screen.scale_y, "OPTIONS");
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 210 * cur_game->screen.scale_y, "EXIT");
}

static void
draw_bg(struct game *cur_game)
{
	int i, j;
	SDL_Rect rect = { 0, 0, cur_game->screen.w, cur_game->screen.h };
	
	/* Have a random chance to change a tile */
	if (rand_num(0, 99) >= 49) {
		/* change one random tile */
		i = rand_num(0, 79);
		j = rand_num(0, 44);
		tile[i][j] = rand_num(512, 559);
		draw_title_tile(cur_game, i, j);
	}

	/* Render the texture */
	SDL_RenderCopy(cur_game->screen.renderer, title_screen_tex, NULL, &rect);
}

static void
save_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int x, y;
	SDL_bool finished;
	SDL_bool *saves;
	SDL_Event event;
	
	saves = check_savefiles();
	
	/* Enter input loop */
	finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw saving screen */
		render_clear(cur_game);
		draw_saveload_screen(cur_game, saves, SDL_TRUE);
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_1: /* save 1 */
					finished = save_all(cur_game, map, cur_player, 0);
					break;
				case SDLK_2: /* save 2 */
					finished = save_all(cur_game, map, cur_player, 1);
					break;
				case SDLK_3: /* save 3 */
					finished = save_all(cur_game, map, cur_player, 2);
					break;
				case SDLK_4: /* save 4 */
					finished = save_all(cur_game, map, cur_player, 3);
					break;
				case SDLK_ESCAPE: /* cancel */
					finished = SDL_TRUE;
					break;
				default:
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			x = event.button.x; y = event.button.y;
			if (x >= 122 * cur_game->screen.scale_x && x <= 343 * cur_game->screen.scale_x) {
				if (y >= 152 * cur_game->screen.scale_y && y <= 163 * cur_game->screen.scale_y) {
					finished = save_all(cur_game, map, cur_player, 0);
				} else if (y >= 172 * cur_game->screen.scale_y && y <= 183 * cur_game->screen.scale_y) {
					finished = save_all(cur_game, map, cur_player, 1);
				} else if (y >= 192 * cur_game->screen.scale_y && y <= 204 * cur_game->screen.scale_y) {
					finished = save_all(cur_game, map, cur_player, 2);
				} else if (y >= 212 * cur_game->screen.scale_y && y <= 224 * cur_game->screen.scale_y) {
					finished = save_all(cur_game, map, cur_player, 3);
				}
			}
		}
	}
}

static SDL_bool
load_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	int x, y;
	SDL_bool finished, loaded;
	SDL_bool *saves;
	SDL_Event event;
	
	saves = check_savefiles();
	
	/* Enter input loop */
	finished = SDL_FALSE;
	loaded = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw loading screen */
		render_clear(cur_game);
		draw_saveload_screen(cur_game, saves, SDL_FALSE);
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_1: /* save 1 */
					if (saves[0] == SDL_TRUE) {
						load_all(cur_game, map, cur_player, 0);
						loaded = SDL_TRUE;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_2: /* save 2 */
					if (saves[1] == SDL_TRUE) {
						load_all(cur_game, map, cur_player, 1);
						loaded = SDL_TRUE;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_3: /* save 3 */
					if (saves[2] == SDL_TRUE) {
						load_all(cur_game, map, cur_player, 2);
						loaded = SDL_TRUE;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_4: /* save 4 */
					if (saves[3] == SDL_TRUE) {
						load_all(cur_game, map, cur_player, 3);
						loaded = SDL_TRUE;
						finished = SDL_TRUE;
					}
					break;
				case SDLK_ESCAPE: /* cancel */
					finished = SDL_TRUE;
					break;
				default:
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			x = event.button.x; y = event.button.y;
			if (x >= 122 * cur_game->screen.scale_x && x <= 343 * cur_game->screen.scale_x) {
				if (y >= 152 * cur_game->screen.scale_y && y <= 163 * cur_game->screen.scale_y) {
					load_all(cur_game, map, cur_player, 0);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 172 * cur_game->screen.scale_x && y <= 183 * cur_game->screen.scale_y) {
					load_all(cur_game, map, cur_player, 1);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 192 * cur_game->screen.scale_x && y <= 204 * cur_game->screen.scale_y) {
					load_all(cur_game, map, cur_player, 2);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 212 * cur_game->screen.scale_x && y <= 224 * cur_game->screen.scale_y) {
					load_all(cur_game, map, cur_player, 3);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				}
			}
		}
	}
	return loaded;
}

static void
opts_screen(struct game *cur_game)
{
	int x, y;
	SDL_bool finished;
	SDL_Event event;
	SDL_bool changed;
	
	/* Enter input loop */
	changed = SDL_FALSE;
	finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw loading screen */
		render_clear(cur_game);
		draw_opts_screen(cur_game);
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE: /* cancel */
					finished = SDL_TRUE;
					break;
				default:
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			x = event.button.x; y = event.button.y;
			if (x >= 100 * cur_game->screen.scale_x && x <= 165 * cur_game->screen.scale_x &&
			    y >= 574 * cur_game->screen.scale_y && y <= 586 * cur_game->screen.scale_y) {
			    	return;
			} else if (x >= 190 * cur_game->screen.scale_x && x <= 346 * cur_game->screen.scale_x) {
				if (y >= 185 * cur_game->screen.scale_y && y <= 199 * cur_game->screen.scale_y &&
				    cur_game->screen.w != 960 && cur_game->screen.h != 540) {
					/* 960x540 */
					cur_game->screen.w = 960;
					cur_game->screen.h = 540;
					cur_game->screen.scale_x = 960.0/1280.0;
					cur_game->screen.scale_y = 960.0/1280.0;
					changed = SDL_TRUE;
				} else if (y >= 214 * cur_game->screen.scale_y && y <= 227 * cur_game->screen.scale_y &&
					   cur_game->screen.w != 1280 && cur_game->screen.h != 720) {
					/* 1280x720 */
					cur_game->screen.w = 1280;
					cur_game->screen.h = 720;
					cur_game->screen.scale_x = 1.0;
					cur_game->screen.scale_y = 1.0;
					changed = SDL_TRUE;
				} else if (y >= 243 * cur_game->screen.scale_y && y <= 258 * cur_game->screen.scale_y &&
					   cur_game->screen.w != 1600 && cur_game->screen.h != 900) {
					/* 1600x900 */
					cur_game->screen.w = 1600;
					cur_game->screen.h = 900;
					cur_game->screen.scale_x = 1600.0/1280.0;
					cur_game->screen.scale_y = 1600.0/1280.0;
					changed = SDL_TRUE;
				} else if (y >= 275 * cur_game->screen.scale_y && y <= 286 * cur_game->screen.scale_y &&
					   cur_game->screen.w != 1920 && cur_game->screen.h != 1080) {
					/* 1920x1080 */
					cur_game->screen.w = 1920;
					cur_game->screen.h = 1080;
					cur_game->screen.scale_x = 1920.0/1280.0;
					cur_game->screen.scale_y = 1920.0/1280.0;
					changed = SDL_TRUE;
				} else if (y >= 334 * cur_game->screen.scale_y && y <= 348 * cur_game->screen.scale_y &&
					   cur_game->screen.fullscreen == SDL_FALSE) {
					/* Window */
					cur_game->screen.fullscreen = SDL_TRUE;
					changed = SDL_TRUE;
				} else if (y >= 364 * cur_game->screen.scale_y && y <= 378 * cur_game->screen.scale_y &&
					   cur_game->screen.fullscreen == SDL_TRUE) {
					cur_game->screen.fullscreen = SDL_FALSE;
					changed = SDL_TRUE;
				} else if (y >= 423 * cur_game->screen.scale_y && y <= 436 * cur_game->screen.scale_y &&
					   cur_game->screen.vsync == SDL_FALSE) {
					/* Vsync on */
					cur_game->screen.vsync = SDL_TRUE;
					changed = SDL_TRUE;
				} else if (y >= 453 * cur_game->screen.scale_y && y <= 467 * cur_game->screen.scale_y &&
					   cur_game->screen.vsync == SDL_TRUE) {
					/* Vsync off */
					cur_game->screen.vsync = SDL_FALSE;
					changed = SDL_TRUE;
				} else if (y >= 514 * cur_game->screen.scale_y && y <= 527 * cur_game->screen.scale_y &&
					   cur_game->screen.scanlines_on == SDL_FALSE) {
					/* Scanlines on */
					cur_game->screen.scanlines_on = SDL_TRUE;
					changed = SDL_TRUE;
				} else if (y >= 543 * cur_game->screen.scale_y && y <= 557 * cur_game->screen.scale_y &&
					   cur_game->screen.scanlines_on == SDL_TRUE) {
					/* Scanlines off */
					cur_game->screen.scanlines_on = SDL_FALSE;
					changed = SDL_TRUE;
				}
			}
		}
		if (changed == SDL_TRUE) {
			/* change video settings - reinitialize display */
			display_quit(cur_game);
			display_init(cur_game);
			changed = SDL_FALSE;
			/* Reset up title screen texture */
			SDL_DestroyTexture(title_screen_tex);
			title_screen_tex = NULL;
			setup_tiles(cur_game);
			save_opts(cur_game);
		}
	}
}

static void
draw_saveload_screen(struct game *cur_game, SDL_bool *saves, SDL_bool save)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* draw save or load header */
	if (save == SDL_TRUE) {
		draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 100 * cur_game->screen.scale_y, "SAVE GAME");
	} else {
		draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 100 * cur_game->screen.scale_y, "LOAD GAME");
	}
	/* draw saves */
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "GAME 1");
	if (saves[0]) draw_small_sentence(cur_game, 220 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 170 * cur_game->screen.scale_y, "GAME 2");
	if (saves[1]) draw_small_sentence(cur_game, 220 * cur_game->screen.scale_x, 170 * cur_game->screen.scale_y, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 190 * cur_game->screen.scale_y, "GAME 3");
	if (saves[2]) draw_small_sentence(cur_game, 220 * cur_game->screen.scale_x, 190 * cur_game->screen.scale_y, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 210 * cur_game->screen.scale_y, "GAME 4");
	if (saves[3]) draw_small_sentence(cur_game, 220 * cur_game->screen.scale_x, 210 * cur_game->screen.scale_y, "SAVE EXISTS");
}

SDL_bool
yesno_screen(struct game *cur_game, char *message)
{
	int x, y;
	SDL_bool finished;
	SDL_bool yes_or_no;
	SDL_Event event;
	
	/* Enter input loop */
	finished = SDL_FALSE;
	yes_or_no = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw loading screen */
		render_clear(cur_game);
		draw_yesno_screen(cur_game, message);
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_y:
					yes_or_no = SDL_TRUE;
					finished = SDL_TRUE;					
					break;
				case SDLK_n:
					yes_or_no = SDL_FALSE;
					finished = SDL_TRUE;					
					break;
				default:
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
			x = event.button.x; y = event.button.y;
			if (x >= 121 * cur_game->screen.scale_x && x <= 154 * cur_game->screen.scale_x &&
			    y >= 152 * cur_game->screen.scale_y && y <= 163 * cur_game->screen.scale_y) {
				yes_or_no = SDL_TRUE;
				finished = SDL_TRUE;
			} else if (x >= 121 * cur_game->screen.scale_x && x <= 141 * cur_game->screen.scale_x &&
				   y >= 173 * cur_game->screen.scale_y && y <= 182 * cur_game->screen.scale_y) {
				yes_or_no = SDL_FALSE;
				finished = SDL_TRUE;
			}
		}
	}
	return yes_or_no;
}

static void
draw_opts_screen(struct game *cur_game)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* draw options header */
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 100 * cur_game->screen.scale_y, "GAME OPTIONS");
	
	/* draw saves */
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "RESOLUTION");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 180 * cur_game->screen.scale_y, "    960x540");
	if (cur_game->screen.w == 960 && cur_game->screen.h == 540) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 180 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 210 * cur_game->screen.scale_y, "    1280x720 (RECOMMENDED)");
	if (cur_game->screen.w == 1280 && cur_game->screen.h == 720) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 210 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 240 * cur_game->screen.scale_y, "    1600x900");
	if (cur_game->screen.w == 1600 && cur_game->screen.h == 900) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 240 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 270 * cur_game->screen.scale_y, "    1920x1080");
	if (cur_game->screen.w == 1920 && cur_game->screen.h == 1080) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 270 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 300 * cur_game->screen.scale_y, "MODE");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 330 * cur_game->screen.scale_y, "    FULLSCREEN");
	if (cur_game->screen.fullscreen == SDL_TRUE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 330 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 360 * cur_game->screen.scale_y, "    WINDOWED (RECOMMENDED)");
	if (cur_game->screen.fullscreen == SDL_FALSE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 360 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 390 * cur_game->screen.scale_y, "VSYNC");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 420 * cur_game->screen.scale_y, "    ON");
	if (cur_game->screen.vsync == SDL_TRUE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 420 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 450 * cur_game->screen.scale_y, "    OFF (RECOMMENDED)");
	if (cur_game->screen.vsync == SDL_FALSE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 450 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 480 * cur_game->screen.scale_y, "SCANLINES");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 510 * cur_game->screen.scale_y, "    ON (RECOMMENDED)");
	if (cur_game->screen.scanlines_on == SDL_TRUE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 510 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 540 * cur_game->screen.scale_y, "    OFF");
	if (cur_game->screen.scanlines_on == SDL_FALSE) draw_sentence(cur_game, 120 * cur_game->screen.scale_x, 540 * cur_game->screen.scale_y, " ->");
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 570 * cur_game->screen.scale_y, "DONE");
	
}

static void
draw_yesno_screen(struct game *cur_game, char *message)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* Draw message */
	draw_sentence(cur_game, 100 * cur_game->screen.scale_x, 100 * cur_game->screen.scale_y, message);
	/* Draw options */
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 150 * cur_game->screen.scale_y, "YES");
	draw_small_sentence(cur_game, 120 * cur_game->screen.scale_x, 170 * cur_game->screen.scale_y, "NO");
}
