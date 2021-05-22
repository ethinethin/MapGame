#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "font.h"
#include "harv.h"
#include "loot.h"
#include "main.h"
#include "maus.h"
#include "play.h"

struct mouse {
	int x;
	int y;
	int offset_x;
	int offset_y;
	SDL_bool mdown;
	int button;
} MOUSE;

struct coords {
	int x;
	int y;
};

/* Function prototypes */
static SDL_bool			move_cursor_qb(struct game *cur_game, int x, int y);
static SDL_bool			move_cursor_inv(struct game *cur_game, int x, int y);
static void			drag_item(struct game *cur_game, struct player *cur_player);
static struct coords		get_click_coordinates(struct player *cur_player);
static void			click_get_item(struct game *cur_game, struct worldmap *map, struct player *cur_player, struct coords pos);
static struct coords		drop_preview(struct game *cur_game, struct player *cur_player);
static void			click_harvest(struct game *cur_game, struct worldmap *map, struct player *cur_player, struct coords pos);

void
mouse_click(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y, unsigned char button)
{
	char start_pos;
	char white[3] = { 255, 255, 255 };
	SDL_bool cursor_moved;
	struct coords pos;
	
	/* Mouse button is pressed down */
	MOUSE.x = x;
	MOUSE.y = y;
	MOUSE.mdown = SDL_TRUE;
	MOUSE.button = button;
	
	/* Move the cursor if necessary */
	if (x >= QB_X && x <= QB_X + QB_W &&
	    y >= QB_Y && y <= QB_Y + QB_H) {
	    	cursor_moved = move_cursor_qb(cur_game, x - QB_X, y - QB_Y);
		if (button == SDL_BUTTON_RIGHT) {
			place_items(cur_game, map, cur_player);
			return;
		}	
	} else if (cur_game->inventory == SDL_TRUE &&
		   x >= INV_X && x <= INV_X + INV_W &&
		   y >= INV_Y && y <= INV_Y + INV_H) {
		cursor_moved = move_cursor_inv(cur_game, x - INV_X, y - INV_Y);
		if (button == SDL_BUTTON_RIGHT) {
			place_items(cur_game, map, cur_player);
			return;
		}
	} else {
		pos = get_click_coordinates(cur_player);
		if (button == SDL_BUTTON_LEFT) {
			click_get_item(cur_game, map, cur_player, pos);
		} else if (button == SDL_BUTTON_RIGHT) {
			click_harvest(cur_game, map, cur_player, pos);
		}	
		cursor_moved = SDL_FALSE;
		return;
	}
	
	/* Enter a user input loop until the mousebutton is lifted */
	if (cur_player->loot[(short int) cur_game->cursor] == 0 || cursor_moved == SDL_FALSE) return;
	
	/* Enter mouse input loop */
	SDL_Event event;
	while(MOUSE.mdown == SDL_TRUE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		drag_item(cur_game, cur_player);
		drop_preview(cur_game, cur_player);
		render_present(cur_game);
		SDL_Delay(10);
		/* poll for an event */
		if (SDL_PollEvent(&event) == 0) continue;
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_UP: /* move up */
					case SDLK_w:
						move_player(cur_game, map, cur_player, 0, -1);
						break;
					case SDLK_DOWN: /* move down */
					case SDLK_s:
						move_player(cur_game, map, cur_player, 0, 1);
						break;
					case SDLK_LEFT: /* move left */
					case SDLK_a:
						move_player(cur_game, map, cur_player, -1, 0);
						break;
					case SDLK_RIGHT: /* move right */
					case SDLK_d:
						move_player(cur_game, map, cur_player, 1, 0);
						break;
					case SDLK_i:
						toggle_inv(cur_game);
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x / cur_game->screen.scale_x;
				MOUSE.y = event.motion.y / cur_game->screen.scale_y;
				break;
			case SDL_MOUSEBUTTONUP:
				MOUSE.mdown = SDL_FALSE;
				break;
		}
	}
	/* Button was let go - did you try to move the item? */
	start_pos = cur_game->cursor;
	if (MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W &&
	    MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) {
		cursor_moved = move_cursor_qb(cur_game, MOUSE.x - QB_X, MOUSE.y - QB_Y);
	} else if (cur_game->inventory == SDL_TRUE &&
		   MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W &&
		   MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H) {
		cursor_moved = move_cursor_inv(cur_game, MOUSE.x - INV_X, MOUSE.y - INV_Y);
	} else {
		/* You tried to drop it */
		pos = drop_preview(cur_game, cur_player);
		handle_throw(cur_game, map, cur_player, pos.x, pos.y, cur_player->quantity[(int) cur_game->cursor]);
		cursor_moved = SDL_FALSE;
	}
	if (cursor_moved == SDL_TRUE) handle_swap(cur_game, cur_player, start_pos);
}

static SDL_bool
move_cursor_qb(struct game *cur_game, int x, int y)
{
	int i;
	for (i = 0; i < 8; i++) {
		if (x > i * SPRITE_W * WIN_SCALE && x <= (i+1) * SPRITE_W * WIN_SCALE) {
			cur_game->cursor = i;
			MOUSE.offset_x = x - i * SPRITE_W * WIN_SCALE;
			MOUSE.offset_y = y;
			return SDL_TRUE;
		}
	}
	return SDL_FALSE;
}

static SDL_bool
move_cursor_inv(struct game *cur_game, int x, int y)
{
	int rows, cols;
	for (cols = 0; cols < 4; cols++) {
		for (rows = 0; rows < 8; rows++) {
			if (x > cols * SPRITE_W * WIN_SCALE && x <= (cols+1) * SPRITE_W * WIN_SCALE &&
				y > rows * SPRITE_H * WIN_SCALE * 1.25 && y <= (rows+1) * SPRITE_H * WIN_SCALE * 1.25) {
				cur_game->cursor = rows+(cols*8)+8;
				MOUSE.offset_x = x - cols * SPRITE_W * WIN_SCALE;
				MOUSE.offset_y = y - rows * SPRITE_H * WIN_SCALE * 1.25;
				return SDL_TRUE;
			}
		}
	}
	return SDL_FALSE;
}

static void
drag_item(struct game *cur_game, struct player *cur_player)
{
	char quantity[4];

	/* Draw item sprite */
	draw_tile(cur_game, MOUSE.x - MOUSE.offset_x, MOUSE.y - MOUSE.offset_y, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
	          get_loot_sprite(cur_player->loot[(short int) cur_game->cursor]), 192);
	/* Draw quantity if item is stackable */
	if (is_loot_stackable(cur_player->loot[(short int) cur_game->cursor]) == UNSTACKABLE) return;
	set_font_alpha(192);
	sprintf(quantity, "%3d", cur_player->quantity[(short int) cur_game->cursor]);
	draw_small_sentence(cur_game, MOUSE.x - MOUSE.offset_x + 30, MOUSE.y - MOUSE.offset_y + SPRITE_H * WIN_SCALE + 2, quantity);
	set_font_alpha(255);
}

static struct coords
get_click_coordinates(struct player *cur_player)
{
	struct coords pos;
	/* get window position based on mouse coords */
	pos.x = (MOUSE.x - GAME_X)/(SPRITE_W * WIN_SCALE);
	pos.y = (MOUSE.y - GAME_Y)/(SPRITE_W * WIN_SCALE);
	/* get position relative to player */
	pos.x -= cur_player->winpos_x;
	pos.y -= cur_player->winpos_y;
	return pos;
}

static void
click_get_item(struct game *cur_game, struct worldmap *map, struct player *cur_player, struct coords pos)
{
	/* Make sure it is close to the player */
	if (pos.x > 1 || pos.x < -1 || pos.y > 1 || pos.y < -1) return;
	/* Try to pick up the item */
	handle_pickup(cur_game, map, cur_player, pos.x, pos.y);
}

static struct coords
drop_preview(struct game *cur_game, struct player *cur_player)
{
	float angle;
	struct coords pos;
	
	/* Should I draw it? */
	if ((MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W &&
	     MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) |
	    (cur_game->inventory == SDL_TRUE &&
	     MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W &&
	     MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H)) {
		pos.x = -255;
		return pos;
	}
	/* Where to draw it */
	pos = get_click_coordinates(cur_player);
	angle = atan2f(-pos.y, pos.x);
	if (angle >= -0.207 && angle < 0.207) {
		pos.x = 1;
		pos.y = 0;
	} else if (angle >= 0.207 && angle < 0.738) {
		pos.x = 1;
		pos.y = -1;
	} else if (angle >= 0.738 && angle < 2.404) {
		pos.x = 0;
		pos.y = -1;
	} else if (angle >= 2.404 && angle < 2.884) {
		pos.x = -1;
		pos.y = -1;
	} else if ((angle >= 2.884 && angle <= M_PI) ||
		   (angle <= -1*M_PI && angle < -3.037)) {
		pos.x = -1;
		pos.y = 0;
	} else if (angle >= -3.307 && angle < -2.404) {
		pos.x = -1;
		pos.y = 1;
	} else if (angle >= -2.404 && angle < -0.738) {
		pos.x = 0;
		pos.y = 1;
	} else if (angle >= -0.738 && angle < -0.207) {
		pos.x = 1;
		pos.y = 1;
	} else {
		pos.x = 0;
		pos.y = 0;
	}
	
	draw_tile(cur_game,
		  GAME_X + cur_player->winpos_x * SPRITE_W * WIN_SCALE + pos.x * SPRITE_W * WIN_SCALE,
		  GAME_Y + cur_player->winpos_y * SPRITE_H * WIN_SCALE + pos.y * SPRITE_H * WIN_SCALE, SPRITE_W * WIN_SCALE, SPRITE_H * WIN_SCALE,
		  get_loot_sprite(cur_player->loot[(short int) cur_game->cursor]), 144);

	return pos;
}

static void
click_harvest(struct game *cur_game, struct worldmap *map, struct player *cur_player, struct coords pos)
{
	/* Make sure it is close to the player */
	if (pos.x > 1 || pos.x < -1 || pos.y > 1 || pos.y < -1) return;
	/* Try to pick up the item */
	harvest_item(cur_game, map, cur_player, cur_player->x+pos.x, cur_player->y+pos.y);
}


void
place_items(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char start_pos;
	char white[3] = { 255, 255, 255 };
	struct coords pos;

	/* Enter input loop */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	SDL_bool cursor_moved = SDL_FALSE;
	start_pos = cur_game->cursor;
	while(finished == SDL_FALSE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		drag_item(cur_game, cur_player);
		drop_preview(cur_game, cur_player);
		render_present(cur_game);
		SDL_Delay(10);
		/* Poll for mouse state */
		if (SDL_PollEvent(&event) == 0) continue;
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_UP: /* move up */
					case SDLK_w:
						move_player(cur_game, map, cur_player, 0, -1);
						break;
					case SDLK_DOWN: /* move down */
					case SDLK_s:
						move_player(cur_game, map, cur_player, 0, 1);
						break;
					case SDLK_LEFT: /* move left */
					case SDLK_a:
						move_player(cur_game, map, cur_player, -1, 0);
						break;
					case SDLK_RIGHT: /* move right */
					case SDLK_d:
						move_player(cur_game, map, cur_player, 1, 0);
						break;
					case SDLK_i:
						toggle_inv(cur_game);
						break;
					default:
						finished = SDL_TRUE;
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x / cur_game->screen.scale_x;
				MOUSE.y = event.motion.y / cur_game->screen.scale_y;
				break;
			case SDL_MOUSEBUTTONUP:
				MOUSE.mdown = SDL_FALSE;
				break;
			case SDL_MOUSEBUTTONDOWN:
				MOUSE.mdown = SDL_TRUE;
				/* Left click, tried to place item */
				if (event.button.button == SDL_BUTTON_LEFT) {
					if (MOUSE.x >= QB_X && MOUSE.x <= QB_X + QB_W &&
					    MOUSE.y >= QB_Y && MOUSE.y <= QB_Y + QB_H) {
						cursor_moved = move_cursor_qb(cur_game, MOUSE.x - QB_X, MOUSE.y - QB_Y);
					} else if (cur_game->inventory == SDL_TRUE &&
						   MOUSE.x >= INV_X && MOUSE.x <= INV_X + INV_W &&
						   MOUSE.y >= INV_Y && MOUSE.y <= INV_Y + INV_H) {
						cursor_moved = move_cursor_inv(cur_game, MOUSE.x - INV_X, MOUSE.y - INV_Y);
					} else {	
						/* You tried to drop it */
						pos = drop_preview(cur_game, cur_player);
						handle_throw(cur_game, map, cur_player, pos.x, pos.y, 1);
						cursor_moved = SDL_FALSE;
					}
					/* Did you try to put item in a different slot? */
					if (cursor_moved == SDL_TRUE) {
						/* Are you clicking the same square? If so, get outta here */
						if (start_pos == cur_game->cursor) {
							finished = SDL_TRUE;
						} else {
							/* Is the loot stackable? */
							if (is_loot_stackable(cur_player->loot[(short int) start_pos]) == SDL_TRUE) {
								/* Is there no item or <255 of the same item? */
								if ((cur_player->loot[(short int) start_pos] == cur_player->loot[(short int) cur_game->cursor] &&
								     cur_player->quantity[(short int) cur_game->cursor] < 255) ||
								     cur_player->loot[(short int) cur_game->cursor] == 0) {
									cur_player->loot[(short int) cur_game->cursor]	= cur_player->loot[(short int) start_pos];
								    	cur_player->quantity[(short int) cur_game->cursor] += 1;
								    	cur_player->quantity[(short int) start_pos] -= 1;
								    	if (cur_player->quantity[(short int) start_pos] == 0) {
								    		cur_player->loot[(short int) start_pos] = 0;
								    	} else {
									    	/* Move cursor back */
										cur_game->cursor = start_pos;
									}
								} else {
									handle_swap(cur_game, cur_player, start_pos);
									finished = SDL_TRUE;
								}
							} else {
								handle_swap(cur_game, cur_player, start_pos);
								finished = SDL_TRUE;
							}
						}
					}
					if (cur_player->quantity[(short int) start_pos] == 0) {
						/* Zero out loot if necessary and you're done */
						cur_player->loot[(short int) start_pos] = 0;
						finished = SDL_TRUE;
					} 
				} else if (event.button.button == SDL_BUTTON_RIGHT) {
					finished = SDL_TRUE;
				}
				break;
		}
	}
}
