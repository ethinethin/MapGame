#include <stdio.h>
#include <SDL2/SDL.h>
#include "disp.h"
#include "loot.h"
#include "main.h"
#include "maus.h"

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
static SDL_bool		move_cursor_qb(struct game *cur_game, int x, int y);
static SDL_bool		move_cursor_inv(struct game *cur_game, int x, int y);
static void		drag_item(struct game *cur_game, struct player *cur_player);
struct coords		get_map_coordinates(struct game *cur_game, struct worldmap *map, struct player *cur_player);

void
mouse_click(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	char start_pos;
	char white[3] = { 255, 255, 255 };
	SDL_bool cursor_moved;
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
		pos = get_map_coordinates(cur_game, map, cur_player);
		cursor_moved = SDL_FALSE;
	}
	
	/* Enter a user input loop until the mousebutton is lifted */
	if (cur_player->loot[(short int) cur_game->cursor] == 0 || cursor_moved == SDL_FALSE) return;
	
	/* output screen to a texture */
	SDL_Texture *texture;
	texture = SDL_CreateTexture(cur_game->screen.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
	SDL_SetRenderTarget(cur_game->screen.renderer, texture);
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	SDL_SetRenderTarget(cur_game->screen.renderer, NULL);
	SDL_Event event;
	SDL_Rect rect = { 0, 0, WIN_W, WIN_H };
	while(MOUSE.mdown == SDL_TRUE && SDL_WaitEvent(&event)) {
		/* Poll for mouse state */
		switch (event.type) {
			case SDL_MOUSEMOTION:
				MOUSE.x = event.motion.x;
				MOUSE.y = event.motion.y;
				/* Draw map from texture */
				SDL_RenderCopy(cur_game->screen.renderer, texture, NULL, &rect); 
				/* move item */
				drag_item(cur_game, cur_player);
				/* Present the screen */
				render_present(cur_game);
				break;
			case SDL_MOUSEBUTTONUP:
				MOUSE.mdown = SDL_FALSE;
				break;
		}
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
	          get_loot_sprite(cur_player->loot[(short int) cur_game->cursor]));
}

struct coords
get_map_coordinates(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	struct coords pos;
	/* get window position based on mouse coords */
	pos.x = (MOUSE.x - GAME_X)/32;
	pos.y = (MOUSE.y - GAME_Y)/32;
	return pos;
	/* blah */
	
	/* get map position based on window position */
	/* blah */
}
