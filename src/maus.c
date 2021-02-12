#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "disp.h"
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
static void			click_get_item(struct worldmap *map, struct player *cur_player, struct coords pos);
static struct coords		drop_preview(struct game *cur_game, struct player *cur_player);

void
mouse_click(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	char start_pos;
	char white[3] = { 255, 255, 255 };
	SDL_bool cursor_moved;
	SDL_Rect rect = { 0, 0, WIN_W, WIN_H };
	struct coords pos;
	
	/* Mouse button is pressed down */
	MOUSE.x = x;
	MOUSE.y = y;
	MOUSE.mdown = SDL_TRUE;
	
	/* Move the cursor if necessary */
	if (x >= QB_X && x <= QB_X + QB_W &&
	    y >= QB_Y && y <= QB_Y + QB_H) {
	    	cursor_moved = move_cursor_qb(cur_game, x - QB_X, y - QB_Y);
	} else if (cur_game->inventory == SDL_TRUE &&
		   x >= INV_X && x <= INV_X + INV_W &&
		   y >= INV_Y && y <= INV_Y + INV_H) {
		cursor_moved = move_cursor_inv(cur_game, x - INV_X, y - INV_Y);
	} else {
		pos = get_click_coordinates(cur_player);
		click_get_item(map, cur_player, pos);
		cursor_moved = SDL_FALSE;
		return;
	}
	
	/* Enter a user input loop until the mousebutton is lifted */
	if (cur_player->loot[(short int) cur_game->cursor] == 0 || cursor_moved == SDL_FALSE) return;
	
	/* output screen to a texture */
	SDL_Texture *texture;
	texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
				    SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
	SDL_SetRenderTarget(cur_game->screen.renderer, texture);
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
	SDL_Event event;
	while(MOUSE.mdown == SDL_TRUE && SDL_WaitEvent(&event)) {
		/* Poll for mouse state */
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
				SDL_DestroyTexture(texture);
				texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888,
							    SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
				SDL_SetRenderTarget(cur_game->screen.renderer, texture);
				draw_game(cur_game, map, cur_player);
				draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
				SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
				break;
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x;
				MOUSE.y = event.motion.y;
				break;
			case SDL_MOUSEBUTTONUP:
				MOUSE.mdown = SDL_FALSE;
				break;
		}
		/* redraw screen */
		SDL_RenderCopy(cur_game->screen.renderer, texture, NULL, &rect); 
		drag_item(cur_game, cur_player);
		drop_preview(cur_game, cur_player);
		render_present(cur_game);
		SDL_Delay(10);
	}
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
	SDL_DestroyTexture(texture);
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
		handle_throw(cur_game, map, cur_player, pos.x, pos.y);
		cursor_moved = SDL_FALSE;
	}
	if (cursor_moved == SDL_TRUE) handle_swap(cur_game, cur_player, start_pos);
}

static SDL_bool
move_cursor_qb(struct game *cur_game, int x, int y)
{
	int i;
	for (i = 0; i < 8; i++) {
		if (x > i*48 && x <= (i+1)*48) {
			cur_game->cursor = i;
			MOUSE.offset_x = x - i*48;
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
			if (x > cols*48 && x <= (cols+1)*48 &&
				y > rows*60 && y <= (rows+1)*60) {
				cur_game->cursor = rows+(cols*8)+8;
				MOUSE.offset_x = x - cols*48;
				MOUSE.offset_y = y - rows*60;
				return SDL_TRUE;
			}
		}
	}
	return SDL_FALSE;
}

static void
drag_item(struct game *cur_game, struct player *cur_player)
{
	draw_tile(cur_game, MOUSE.x - MOUSE.offset_x, MOUSE.y - MOUSE.offset_y, 48, 48,
	          get_loot_sprite(cur_player->loot[(short int) cur_game->cursor]), 192);
}

static struct coords
get_click_coordinates(struct player *cur_player)
{
	struct coords pos;
	/* get window position based on mouse coords */
	pos.x = (MOUSE.x - GAME_X)/32;
	pos.y = (MOUSE.y - GAME_Y)/32;
	/* get position relative to player */
	pos.x -= cur_player->winpos_x;
	pos.y -= cur_player->winpos_y;
	return pos;
}

static void
click_get_item(struct worldmap *map, struct player *cur_player, struct coords pos)
{
	/* Make sure it is close to the player */
	if (pos.x > 1 || pos.x < -1 || pos.y > 1 || pos.y < -1) return;
	/* Try to pick up the item */
	handle_pickup(map, cur_player, pos.x, pos.y);
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
		  GAME_X + cur_player->winpos_x*32 + pos.x * 32,
		  GAME_Y + cur_player->winpos_y*32 + pos.y * 32, 32, 32,
		  get_loot_sprite(cur_player->loot[(short int) cur_game->cursor]), 144);

	return pos;
}
