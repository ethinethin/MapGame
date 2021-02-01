#include <stdlib.h>
#include "disp.h"
#include "font.h"
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

/* Function prototypes */
static SDL_bool		handle_pickup(struct worldmap *map, struct player *cur_player, int dir);
static SDL_bool		handle_throw(struct game *cur_game, struct worldmap *map, struct player *cur_player, int dir);

struct loot LOOT[9] = {
	{NULL, 0, UNSTACKABLE, PASSABLE},
	{"money", 258, STACKABLE, PASSABLE},
	{"potion", 259, STACKABLE, PASSABLE},
	{"sword", 261, UNSTACKABLE, PASSABLE},
	{"shield", 262, UNSTACKABLE, PASSABLE},
	{"key", 263, STACKABLE, PASSABLE},
	{"necklace", 268, UNSTACKABLE, PASSABLE},
	{"chicken", 271, STACKABLE, PASSABLE},
	{"wall", 78, UNSTACKABLE, IMPASSABLE}
};

short int
get_loot_sprite(short int id)
{
	return LOOT[id].sprite;
}

char
is_loot_passable(short int id)
{
	return LOOT[id].passable;
}

char *
get_loot_name(short int id)
{
	return LOOT[id].name;
}

char
is_loot_stackable(short int id)
{
	return LOOT[id].stackable;
}

void
pickup_item(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };

	/* Prompt for direction and render */
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
	draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
	render_present(cur_game);
	
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while(finished == SDL_FALSE && SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_UP: /* pickup up */
				case SDLK_w:
					finished = handle_pickup(map, cur_player, 0);
					break;
				case SDLK_RIGHT: /* pickup right */
				case SDLK_d:
					finished = handle_pickup(map, cur_player, 1);
					break;
				case SDLK_DOWN: /* pickup down */
				case SDLK_s:
					finished = handle_pickup(map, cur_player, 2);
					break;
				case SDLK_LEFT: /* pickup left */
				case SDLK_a:
					finished = handle_pickup(map, cur_player, 3);
					break;
				default:
					finished = SDL_TRUE;
					return;
					break;
			}
		}
	}
}

static SDL_bool
handle_pickup(struct worldmap *map, struct player *cur_player, int dir)
{
	unsigned char cur_quantity;
	short int cur_item;
	int i;
	int new_x = cur_player->x;
	int new_y = cur_player->y;

	/* Are you at the map boundaries? */
	if ((cur_player->x == 0 && dir == 3) ||
	    (cur_player->x == map->col_size - 1 && dir == 1) ||
	    (cur_player->y == 0 && dir == 0) ||
	    (cur_player->y == map->row_size -1 && dir == 2)) {
	    	return SDL_TRUE;
	}

	/* Is there an item on the map? */
	switch (dir) {
		case 0:
			new_y -= 1;
			break;
		case 1:
			new_x += 1;
			break;
		case 2:
			new_y += 1;
			break;
		case 3:
			new_x -= 1;
			break;
		default:
			break;
	}
	cur_item = *(*(map->loot+new_y)+new_x);
	if (cur_item == 0) {
		return SDL_TRUE;
	} else {
		cur_quantity = *(*(map->quantity+new_y)+new_x);
	}
	
	/* If it's stackable, check for a non-maxed stack in the inventory */
	if (is_loot_stackable(cur_item) == STACKABLE) {
		for (i = 0; i < 40; i++) {
			if (cur_player->loot[i] == cur_item && cur_player->quantity[i] < 255) {
				if ((int) cur_quantity + (int) cur_player->quantity[i] > 255) {
					cur_quantity -= 255 - cur_player->quantity[i];
					*(*(map->quantity+new_y)+new_x) -= 255 - cur_player->quantity[i];
					cur_player->quantity[i] = 255;
				} else {
					cur_player->quantity[i] += cur_quantity;
					*(*(map->loot+new_y)+new_x) = 0;
					*(*(map->quantity+new_y)+new_x) = 0;
					return SDL_TRUE;
				}		
			}
		}
	}
	/* There's still item left (or the item was unstackable) */
	if (cur_quantity > 0) {
		for (i = 0; i < 40; i++) {
			if (cur_player->loot[i] == 0) {
				cur_player->loot[i] = cur_item;
				cur_player->quantity[i] = cur_quantity;
				*(*(map->loot+new_y)+new_x) = 0;
				*(*(map->quantity+new_y)+new_x) = 0;
				return SDL_TRUE;
			}
		}
	}
	return SDL_TRUE;
}

void
throw_item(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };

	/* Check for item */
	if (cur_player->loot[(int) cur_game->cursor] == 0) return;

	/* Prompt for direction and render */
	draw_game(cur_game, map, cur_player);
	draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
	draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
	draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
	render_present(cur_game);
	
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while(finished == SDL_FALSE && SDL_WaitEvent(&event)) {
		if (event.type == SDL_KEYDOWN) {
			switch(event.key.keysym.sym) {
				case SDLK_UP: /* throw up */
				case SDLK_w:
					finished = handle_throw(cur_game, map, cur_player, 0);
					break;
				case SDLK_RIGHT: /* throw right */
				case SDLK_d:
					finished = handle_throw(cur_game, map, cur_player, 1);
					break;
				case SDLK_DOWN: /* throw down */
				case SDLK_s:
					finished = handle_throw(cur_game, map, cur_player, 2);
					break;
				case SDLK_LEFT: /* throw left */
				case SDLK_a:
					finished = handle_throw(cur_game, map, cur_player, 3);
					break;
				default:
					finished = SDL_TRUE;
					return;
					break;
			}
		}
	}
}

static SDL_bool
handle_throw(struct game *cur_game, struct worldmap *map, struct player *cur_player, int dir)
{
	int new_x = cur_player->x;
	int new_y = cur_player->y;
	short int item_on_map;
	short int cur_item;
	
	/* Check item is ok to throw */
	if ((cur_player->x == 0 && dir == 3) ||
	    (cur_player->x == map->col_size - 1 && dir == 1) ||
	    (cur_player->y == 0 && dir == 0) ||
	    (cur_player->y == map->row_size -1 && dir == 2)) {
	    	return SDL_TRUE;
	}
	
	/* Figure out where to put it */
	switch (dir) {
		case 0:
			new_y -= 1;
			break;
		case 1:
			new_x += 1;
			break;
		case 2:
			new_y += 1;
			break;
		case 3:
			new_x -= 1;
			break;
		default:
			break;
	}
	
	/* is there an item on the map already? */
	item_on_map = *(*(map->loot+new_y)+new_x);
	cur_item = cur_player->loot[(int) cur_game->cursor];
	/* There's no item on map, just place it and leave */
	if (item_on_map == 0) {
		*(*(map->loot+new_y)+new_x) = cur_item;
		*(*(map->quantity+new_y)+new_x) = cur_player->quantity[(int) cur_game->cursor];
		cur_player->loot[(int) cur_game->cursor] = 0;
		cur_player->quantity[(int) cur_game->cursor] = 0;
		return SDL_TRUE;
	} else {
		/* Unstackable item blocking map, get outta here */
		if (is_loot_stackable(item_on_map) == UNSTACKABLE) {
			return SDL_TRUE;
		} else if (item_on_map != cur_item) {
			return SDL_TRUE;
		}
	}
	
	/* We only get here if there's an item on the map that is stackable equal to what we're trying to drop */
	/* Will the stack overflow? */
	unsigned char drop_how_much;
	if ((int) *(*(map->quantity+new_y)+new_x) + (int) cur_player->quantity[(int) cur_game->cursor] > 255) {
		drop_how_much = 255 - *(*(map->quantity+new_y)+new_x);
	} else {
		drop_how_much = cur_player->quantity[(int) cur_game->cursor];
	}
	/* Make the change */
	*(*(map->loot+new_y)+new_x) = cur_item;
	*(*(map->quantity+new_y)+new_x) += drop_how_much;
	cur_player->quantity[(int) cur_game->cursor] -= drop_how_much;
	
	/* Any items left? */
	if (cur_player->quantity[(int) cur_game->cursor] == 0) {
		cur_player->loot[(int) cur_game->cursor] = 0;
	}
	
	return SDL_TRUE;
}

void
move_cursor(struct game *cur_game, char dir)
{
	cur_game->cursor += dir;
	if (cur_game->cursor < 0) {
		cur_game->cursor = 7;
	} else if (cur_game->cursor > 7) {
		cur_game->cursor = 0;
	}
}

void
move_cursor_click(struct game *cur_game, int x)
{
	int i;
	for (i = 0; i < 8; i++) {
		if (x > i*48 && x <= (i+1)*48) {
			cur_game->cursor = i;
			return;
		}
	}
}
