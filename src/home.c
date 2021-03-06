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
static void		generate_farts(struct game *cur_game, struct worldmap *main_map);
static void		copy_fart(struct worldmap *main_map, struct worldmap *fart, int row, int col);
static void		setup_tiles(struct game *cur_game);
static void		draw_title_tiles(struct game *cur_game);
static void		draw_title_tile(struct game *cur_game, int i, int j);
static void		draw_title_screen(struct game *cur_game, int status);
static void		draw_bg(struct game *cur_game);
static void		save_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static SDL_bool		load_screen(struct game *cur_game, struct worldmap *map, struct player *cur_player);
static void		opts_screen(struct game *cur_game);
static void		draw_saveload_screen(struct game *cur_game, SDL_bool *saves, SDL_bool save);
static void		draw_opts_screen(struct game *cur_game);
static void		draw_yesno_screen(struct game *cur_game, char *message);
static SDL_bool		change_resolution(struct game *cur_game, int w, int h);

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
			x = event.button.x / cur_game->screen.scale_x;
			y = event.button.y / cur_game->screen.scale_y;
			if (x >= 123 && x <= 219 && y >= 153 && y <= 162) {
				/* new game or save game */
				if (status == STARTING_GAME) {
					new_game = STARTING_GAME;
					finished = SDL_TRUE;
				} else if (status == GAME_IN_PROGRESS) {
					new_game = GAME_IN_PROGRESS;
					finished = SDL_TRUE;
					save_screen(cur_game, map, cur_player);
				}
			} else if (x >= 123 && x <= 219 && y >= 175 && y <= 183) {
				/* continue */
				if (status == STARTING_GAME) {
					new_game = CONTINUE_GAME;
					finished = load_screen(cur_game, map, cur_player);
				} else {
					new_game = GAME_IN_PROGRESS;
					finished = SDL_TRUE;
				}
			} else if (x >= 123 && x <= 195 && y >= 193 && y <= 203) {
				/* options */
				opts_screen(cur_game);
				continue;
			} else if (x >= 123 && x <= 161 && y >= 213 && y <= 222) {
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
		/* In case display settings have changes */
		cur_game->screen.display_reloaded = SDL_FALSE;
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
						 SDL_TEXTUREACCESS_TARGET, 1280, 720);
		SDL_SetTextureBlendMode(title_screen_tex, SDL_BLENDMODE_BLEND);
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
	draw_rect(cur_game, 0, 0, 1280, 720, SDL_TRUE, black, SDL_FALSE, NULL);
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 45; j++) {
			draw_tile(cur_game, i * 16, j * 16,
				  16, 16, tile[i][j], 128);
		}
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->screen.output);
}

static void
draw_title_tile(struct game *cur_game, int i, int j)
{
	char black[3] = { 0, 0, 0 };
	
	/* Render to texture */
	SDL_SetRenderTarget(cur_game->screen.renderer, title_screen_tex);
	draw_rect(cur_game, i * 16, j * 16, 16, 16, SDL_TRUE, black, SDL_FALSE, NULL);
	draw_tile(cur_game, i * 16, j * 16, 16, 16, tile[i][j], 128);
	SDL_SetRenderTarget(cur_game->screen.renderer, cur_game->screen.output);
}

static void
draw_title_screen(struct game *cur_game, int status)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* draw game title */
	draw_sentence(cur_game, 100, 100, "MAPGAME");
	/* draw new game */
	if (status == STARTING_GAME) {
		draw_small_sentence(cur_game, 120, 150, "NEW GAME");
	} else {
		draw_small_sentence(cur_game, 120, 150, "SAVE GAME");
	}
	/* draw continue */
	if (status == STARTING_GAME) {
		draw_small_sentence(cur_game, 120, 170, "LOAD GAME");
	} else {
		draw_small_sentence(cur_game, 120, 170, "CONTINUE");
	}
	draw_small_sentence(cur_game, 120, 190, "OPTIONS");
	draw_small_sentence(cur_game, 120, 210, "EXIT");
}

static void
draw_bg(struct game *cur_game)
{
	int i, j;
	SDL_Rect rect = { 0, 0, 1280, 720 };
	
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
			x = event.button.x / cur_game->screen.scale_x;
			y = event.button.y / cur_game->screen.scale_y;
			if (x >= 122 && x <= 343) {
				if (y >= 152 && y <= 163) {
					finished = save_all(cur_game, map, cur_player, 0);
				} else if (y >= 172 && y <= 183) {
					finished = save_all(cur_game, map, cur_player, 1);
				} else if (y >= 192 && y <= 204) {
					finished = save_all(cur_game, map, cur_player, 2);
				} else if (y >= 212 && y <= 224) {
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
			x = event.button.x / cur_game->screen.scale_x;
			y = event.button.y / cur_game->screen.scale_y;
			if (x >= 122 && x <= 343) {
				if (y >= 152 && y <= 163 && saves[0] == SDL_TRUE) {
					load_all(cur_game, map, cur_player, 0);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 172 && y <= 183 && saves[1] == SDL_TRUE) {
					load_all(cur_game, map, cur_player, 1);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 192 && y <= 204 && saves[2] == SDL_TRUE) {
					load_all(cur_game, map, cur_player, 2);
					loaded = SDL_TRUE;
					finished = SDL_TRUE;
				} else if (y >= 212 && y <= 224 && saves[3] == SDL_TRUE) {
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
			x = event.button.x / cur_game->screen.scale_x;
			y = event.button.y / cur_game->screen.scale_y;
			if (x >= 101 && x <= 275) {
				/* Clicked on main display options */
				if (y >= 180 && y <= 195) {
					/* fullscreen */
					if (cur_game->screen.displaymode == 2) {
						cur_game->screen.displaymode = 0;
						changed = SDL_TRUE;
					}
				} else if (y >= 200 && y <= 215) {
					/* window */
					if (cur_game->screen.displaymode != 2) {
						cur_game->screen.displaymode = 2;
						changed = SDL_TRUE;
					}
				} else if (y >= 240 && y <= 255) {
					/* vsync on */
					if (cur_game->screen.vsync == SDL_FALSE) {
						cur_game->screen.vsync = SDL_TRUE;
						changed = SDL_TRUE;
					}
				} else if (y >= 260 && y <= 275) {
					/* vsync off */
					if (cur_game->screen.vsync == SDL_TRUE) {
						cur_game->screen.vsync = SDL_FALSE;
						changed = SDL_TRUE;
					}
				} else if (y >= 300 && y <= 315) {
					/* scanlines on */
					if (cur_game->screen.scanlines_on == SDL_FALSE) {
						cur_game->screen.scanlines_on = SDL_TRUE;
					}
				} else if (y >= 320 && y <= 335) {
					/* scanlines off */
					if (cur_game->screen.scanlines_on == SDL_TRUE) {
						cur_game->screen.scanlines_on = SDL_FALSE;
					}
				}	
			} else if (x >= 420 && x <= 545) {
				/* Clicked on resolutions */
				if (y >= 180 && y <= 195) {
					/* Default */
					if (cur_game->screen.displaymode == 1) {
						cur_game->screen.displaymode = 0;
						changed = SDL_TRUE;
					} else if (cur_game->screen.displaymode == 2) {
						changed = change_resolution(cur_game, 1280, 720);
					}
				} else if (y >= 200 && y <= 215) {
					/* 1280x720 */
					changed = change_resolution(cur_game, 1280, 720);
				} else if (y >= 220 && y <= 235) {
					/* 1360x768 */
					changed = change_resolution(cur_game, 1360, 768);
				} else if (y >= 240 && y <= 255) {
					/* 1366x768 */
					changed = change_resolution(cur_game, 1366, 768);
				} else if (y >= 260 && y <= 275) {
					/* 1440x900 */
					changed = change_resolution(cur_game, 1440, 900);
				} else if (y >= 280 && y <= 295) {
					/* 1600x900 */
					changed = change_resolution(cur_game, 1600, 900);
				} else if (y >= 300 && y <= 315) {
					/* 1680x1050 */
					changed = change_resolution(cur_game, 1680, 1050);
				} else if (y >= 320 && y <= 335) {
					/* 1920x1080 */
					changed = change_resolution(cur_game, 1920, 1080);
				} else if (y >= 340 && y <= 355) {
					/* 1920x1200 */
					changed = change_resolution(cur_game, 1920, 1200);
				} else if (y >= 360 && y <= 375) {
					/* 2560x1440 */
					changed = change_resolution(cur_game, 2560, 1440);
				} else if (y >= 380 && y <= 395) {
					/* 2560x1600 */
					changed = change_resolution(cur_game, 2560, 1600);
				} else if (y >= 400 && y <= 415) {
					/* 3840x2160 */
					changed = change_resolution(cur_game, 3840, 2160);
				} 
			}
		}
		if (changed == SDL_TRUE) {
			/* Reset up title screen texture */
			SDL_DestroyTexture(title_screen_tex);
			title_screen_tex = NULL;
			/* change video settings - reinitialize display */
			display_quit(cur_game);
			display_init(cur_game);
			changed = SDL_FALSE;
			setup_tiles(cur_game);
			save_opts(cur_game);
			/* signal that display settings have changed */
			cur_game->screen.display_reloaded = SDL_TRUE;
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
		draw_sentence(cur_game, 100, 100, "SAVE GAME");
	} else {
		draw_sentence(cur_game, 100, 100, "LOAD GAME");
	}
	/* draw saves */
	draw_small_sentence(cur_game, 120, 150, "GAME 1");
	if (saves[0]) draw_small_sentence(cur_game, 220, 150, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120, 170, "GAME 2");
	if (saves[1]) draw_small_sentence(cur_game, 220, 170, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120, 190, "GAME 3");
	if (saves[2]) draw_small_sentence(cur_game, 220, 190, "SAVE EXISTS");
	draw_small_sentence(cur_game, 120, 210, "GAME 4");
	if (saves[3]) draw_small_sentence(cur_game, 220, 210, "SAVE EXISTS");
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
			x = event.button.x / cur_game->screen.scale_x;
			y = event.button.y / cur_game->screen.scale_y;
			if (x >= 121 && x <= 154 && y >= 152 && y <= 163) {
				yes_or_no = SDL_TRUE;
				finished = SDL_TRUE;
			} else if (x >= 121 && x <= 141 && y >= 173 && y <= 182) {
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
	draw_sentence(cur_game, 100, 100, "DISPLAY OPTIONS");
	
	/* draw display options */
	draw_small_sentence(cur_game, 100, 160, "DISPLAY MODE");
	draw_small_sentence(cur_game, 120, 180, "    FULLSCREEN");
	if (cur_game->screen.displaymode == 0 || cur_game->screen.displaymode == 1) draw_small_sentence(cur_game, 120, 180, "  >");
	draw_small_sentence(cur_game, 120, 200, "    WINDOW");
	if (cur_game->screen.displaymode == 2) draw_small_sentence(cur_game, 120, 200, "  >");
	draw_small_sentence(cur_game, 100, 220, "VSYNC");
	draw_small_sentence(cur_game, 120, 240, "    ON");
	if (cur_game->screen.vsync == SDL_TRUE) draw_small_sentence(cur_game, 120, 240, "  >");
	draw_small_sentence(cur_game, 120, 260, "    OFF");
	if (cur_game->screen.vsync == SDL_FALSE) draw_small_sentence(cur_game, 120, 260, "  >");
	draw_small_sentence(cur_game, 100, 280, "SCANLINES");
	draw_small_sentence(cur_game, 120, 300, "    ON");
	if (cur_game->screen.scanlines_on == SDL_TRUE) draw_small_sentence(cur_game, 120, 300, "  >");
	draw_small_sentence(cur_game, 120, 320, "    OFF");
	if (cur_game->screen.scanlines_on == SDL_FALSE) draw_small_sentence(cur_game, 120, 320, "  >");
	/* draw resolution options */
	draw_small_sentence(cur_game, 420, 160, "RESOLUTIONS");
	draw_small_sentence(cur_game, 420, 180, "    DEFAULT");
	draw_small_sentence(cur_game, 420, 200, "    1280x720");
	if (cur_game->screen.w == 1280 && cur_game->screen.h == 720) draw_small_sentence(cur_game, 420, 200, "  >");
	draw_small_sentence(cur_game, 420, 220, "    1360x768");
	if (cur_game->screen.w == 1360 && cur_game->screen.h == 768) draw_small_sentence(cur_game, 420, 220, "  >");
	draw_small_sentence(cur_game, 420, 240, "    1366x768");
	if (cur_game->screen.w == 1366 && cur_game->screen.h == 768) draw_small_sentence(cur_game, 420, 240, "  >");
	draw_small_sentence(cur_game, 420, 260, "    1440x900");
	if (cur_game->screen.w == 1440 && cur_game->screen.h == 900) draw_small_sentence(cur_game, 420, 260, "  >");
	draw_small_sentence(cur_game, 420, 280, "    1600x900");
	if (cur_game->screen.w == 1600 && cur_game->screen.h == 900) draw_small_sentence(cur_game, 420, 280, "  >");
	draw_small_sentence(cur_game, 420, 300, "    1680x1050");
	if (cur_game->screen.w == 1680 && cur_game->screen.h == 1050) draw_small_sentence(cur_game, 420, 300, "  >");
	draw_small_sentence(cur_game, 420, 320, "    1920x1080");
	if (cur_game->screen.w == 1920 && cur_game->screen.h == 1080) draw_small_sentence(cur_game, 420, 320, "  >");
	draw_small_sentence(cur_game, 420, 340, "    1920x1200");
	if (cur_game->screen.w == 1920 && cur_game->screen.h == 1200) draw_small_sentence(cur_game, 420, 340, "  >");
	draw_small_sentence(cur_game, 420, 360, "    2560x1440");
	if (cur_game->screen.w == 2560 && cur_game->screen.h == 1440) draw_small_sentence(cur_game, 420, 360, "  >");
	draw_small_sentence(cur_game, 420, 380, "    2560x1600");
	if (cur_game->screen.w == 2560 && cur_game->screen.h == 1600) draw_small_sentence(cur_game, 420, 380, "  >");
	draw_small_sentence(cur_game, 420, 400, "    3840x2160");
	if (cur_game->screen.w == 3840 && cur_game->screen.h == 2160) draw_small_sentence(cur_game, 420, 400, "  >");
}

static void
draw_yesno_screen(struct game *cur_game, char *message)
{
	/* Draw background */
	draw_bg(cur_game);	
	/* Draw message */
	draw_sentence(cur_game, 100, 100, message);
	/* Draw options */
	draw_small_sentence(cur_game, 120, 150, "YES");
	draw_small_sentence(cur_game, 120, 170, "NO");
}

static SDL_bool
change_resolution(struct game *cur_game, int w, int h)
{
	if (cur_game->screen.w == w && cur_game->screen.h == h) {
		/* Clicking on already-set resolution */
		return SDL_FALSE;
	} else {
		/* If they are in fullscreen desktop mode, change to fullscreen mode */
		if (cur_game->screen.displaymode == 0) {
			cur_game->screen.displaymode = 1;
		}
		cur_game->screen.w = w;
		cur_game->screen.h = h;
		return SDL_TRUE;
	}
}
