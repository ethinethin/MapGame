#include <stdlib.h>
#include "disp.h"
#include "font.h"
#include "hold.h"
#include "loot.h"
#include "main.h"
#include "make.h"
#include "maps.h"
#include "maus.h"
#include "play.h"

struct loot LOOT[] = {
	{NULL, 0, UNSTACKABLE, PASSABLE, NOITEM},
	{"STONE WALL", 576, STACKABLE, IMPASSABLE, WALL},
	{"STONE FLOOR", 575, STACKABLE, PASSABLE, GROUND},
	{"SLATE ROOF", 579, STACKABLE, PASSABLE, ROOF},
	{"STONE DOOR", 577, STACKABLE, IMPASSABLE, C_DOOR},
	{"stone door (open)", 578, STACKABLE, PASSABLE, O_DOOR},
	{"WOODEN CHEST", 580, STACKABLE, IMPASSABLE, HOLDER},
	{"RED CHEST", 581, STACKABLE, IMPASSABLE, HOLDER},
	{"BLUE CHEST", 582, STACKABLE, IMPASSABLE, HOLDER},
	{"FISHING POLE", 583, STACKABLE, PASSABLE, ITEM},
	{"nuthin1", 0, STACKABLE, PASSABLE, ITEM},
	{"nuthin2", 0, STACKABLE, PASSABLE, ITEM},
	{"nuthin3", 0, STACKABLE, PASSABLE, ITEM},
	{"nuthin4", 0, STACKABLE, PASSABLE, ITEM},
	/* grassland sprites */
	{"grass weeds", 560, STACKABLE, PASSABLE, ITEM},//14
	{"grass seeds", 561, STACKABLE, PASSABLE, ITEM},
	{"BEETLE HUSK", 562, STACKABLE, PASSABLE, ITEM},
	{"live beetle", 563, STACKABLE, PASSABLE, ITEM},
	{"STONE", 564, STACKABLE, PASSABLE, ITEM},
	{"SPIDER SILK", 565, STACKABLE, PASSABLE, ITEM},
	{"live spider", 566, STACKABLE, PASSABLE, ITEM},
	{"BRANCH", 567, STACKABLE, PASSABLE, ITEM},
	{"flint", 568, STACKABLE, PASSABLE, ITEM},
	{"worm", 569, STACKABLE, PASSABLE, ITEM},
	{"SLATE", 570, STACKABLE, PASSABLE, ITEM},
	{"grassland ore", 571, STACKABLE, PASSABLE, ITEM},
	{"grassland gem", 572, STACKABLE, PASSABLE, ITEM},
	{"fly", 573, STACKABLE, PASSABLE, ITEM},
	{"smooth stone", 574, STACKABLE, PASSABLE, ITEM}
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

char
get_loot_type(short int id)
{
	return LOOT[id].type;
}

void
pickup_item(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
		draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
		render_present(cur_game);
		SDL_Delay(10);
		/* Poll for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_UP: /* pickup up */
				case SDLK_w:
					finished = handle_pickup(cur_game, map, cur_player, 0, -1);
					break;
				case SDLK_RIGHT: /* pickup right */
				case SDLK_d:
					finished = handle_pickup(cur_game, map, cur_player, 1, 0);
					break;
				case SDLK_DOWN: /* pickup down */
				case SDLK_s:
					finished = handle_pickup(cur_game, map, cur_player, 0, 1);
					break;
				case SDLK_LEFT: /* pickup left */
				case SDLK_a:
					finished = handle_pickup(cur_game, map, cur_player, -1, 0);
					break;
				case SDLK_i:
					toggle_inv(cur_game);
					break;
				default:
					finished = SDL_TRUE;
					break;
			}
		}
	}
	check_recipes(cur_player);
}

SDL_bool
handle_pickup(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	unsigned short int cur_quantity;
	char loot_type;
	unsigned short int *cur_item;
	int i;
	int new_x; 
	int new_y; 

	/* Set pickup coordinates */
	new_x = cur_player->x + x;
	new_y = cur_player->y + y;
	
	/* Are you at the map boundaries? */
	if (new_x < 0 || new_x > map->col_size - 1 ||
	    new_y < 0 || new_y > map->row_size - 1) {
	    	return SDL_TRUE;
	}

	/* Is there a roof tile? */
	cur_item = *(map->roof+new_y)+new_x;
	cur_quantity = 1;
	loot_type = ROOF;
	if (*cur_item == 0 || *(*(map->roof+cur_player->y)+cur_player->x) != 0) {
		/* No, or player is inside, so point to item */
		cur_item = *(map->loot+new_y)+new_x;
		cur_quantity = *(*(map->quantity+new_y)+new_x);
		loot_type = get_loot_type(*cur_item);
	}
	/* Is there an item? */
	if (*cur_item == 0) {
		/* No, point to ground */
		cur_item = *(map->ground+new_y)+new_x;
		cur_quantity = 1;
		loot_type = GROUND;
	}
	/* Is there a ground? */
	if (*cur_item == 0) {
		/* No, get up on out of here */
		return SDL_TRUE;
	}
	
	/* Is it a placed door? */
	if (cur_quantity == 1 && (loot_type == C_DOOR || loot_type == O_DOOR)) {
		switch(loot_type) {
			case C_DOOR:
				/* open it */
				*cur_item += 1;
				break;
			case O_DOOR:
				/* close it */
				*cur_item -= 1;
				break;
		}
		return SDL_TRUE;
	}
	
	/* Is it a chest? */
	if (cur_quantity == 1 && loot_type == HOLDER) {
		open_chest(cur_game, map, cur_player, new_x, new_y);
		return SDL_TRUE;
	}
	
	/* If it's stackable, check for a non-maxed stack in the inventory */
	if (is_loot_stackable(*cur_item) == STACKABLE) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == *cur_item && cur_player->quantity[i] < MAX_STACK) {
				if ((int) cur_quantity + (int) cur_player->quantity[i] > MAX_STACK) {
					cur_quantity -= MAX_STACK - cur_player->quantity[i];
					cur_player->quantity[i] = MAX_STACK;
					if (loot_type != ROOF && loot_type != GROUND) {
						*(*(map->quantity+new_y)+new_x) -= MAX_STACK - cur_player->quantity[i];
					}
				} else {
					cur_player->quantity[i] += cur_quantity;
					*cur_item = 0;
					if (loot_type != ROOF && loot_type != GROUND) {
						*(*(map->quantity+new_y)+new_x) = 0;
					}
					return SDL_TRUE;
				}		
			}
		}
	}
	/* There's still item left (or the item was unstackable) */
	if (cur_quantity > 0) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == 0) {
				cur_player->loot[i] = *cur_item;
				cur_player->quantity[i] = cur_quantity;
				*cur_item = 0;
				if (loot_type != ROOF && loot_type != GROUND) {
					*(*(map->quantity+new_y)+new_x) = 0;
				}
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
	
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*16 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
		draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Which direction?");
		render_present(cur_game);
		SDL_Delay(10);
		/* Check for user input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_UP: /* throw up */
				case SDLK_w:
					finished = handle_throw(cur_game, map, cur_player, 0, -1, cur_player->quantity[(int) cur_game->cursor]);
					break;
				case SDLK_RIGHT: /* throw right */
				case SDLK_d:
					finished = handle_throw(cur_game, map, cur_player, 1, 0, cur_player->quantity[(int) cur_game->cursor]);
					break;
				case SDLK_DOWN: /* throw down */
				case SDLK_s:
					finished = handle_throw(cur_game, map, cur_player, 0, 1, cur_player->quantity[(int) cur_game->cursor]);
					break;
				case SDLK_LEFT: /* throw left */
				case SDLK_a:
					finished = handle_throw(cur_game, map, cur_player, -1, 0, cur_player->quantity[(int) cur_game->cursor]);
					break;
				case SDLK_i:
					toggle_inv(cur_game);
					break;
				default:
					finished = SDL_TRUE;
					return;
					break;
			}
		}
	}
}

SDL_bool
handle_throw(struct game *cur_game, struct worldmap *map, struct player *cur_player,  int x, int y, short int quantity)
{
	char loot_type;
	int new_x;
	int new_y;
	short int item_on_map;
	short int cur_item;
	short int cur_quantity;
	
	/* Set pickup coordinates */
	new_x = cur_player->x + x;
	new_y = cur_player->y + y;
	
	/* Are you at the map boundaries? */
	if (new_x < 0 || new_x > map->col_size - 1 ||
	    new_y < 0 || new_y > map->row_size - 1) {
	    	return SDL_TRUE;
	}
	
	/* Is the current map tile IMPASSABLE? */
	if (is_passable(*(*(map->tile+new_y)+new_x), *(*(map->biome+new_y)+new_x)) == IMPASSABLE) {
		return SDL_TRUE;
	}
		
	/* What's the item being dropped? */
	cur_item = cur_player->loot[(int) cur_game->cursor];
	cur_quantity = *(*(map->quantity+new_y)+new_x);
	loot_type = get_loot_type(cur_item);

	/* Is there a single DOOR or CONTAINER? */
	if (cur_quantity == 1 && (loot_type == C_DOOR || loot_type == O_DOOR || loot_type == HOLDER)) {
		return SDL_TRUE;
	}

	/* Is it a ground tile or item? quantity = amount dropped */
	if (loot_type == GROUND && quantity == 1) {
		/* Is there already a ground tile? */
		if (*(*(map->ground+new_y)+new_x) != 0) {
			return SDL_TRUE;
		}
		/* There's no ground, place the ground, remove from inventory and leave */
		*(*(map->ground+new_y)+new_x) = cur_item;
		cur_player->quantity[(int) cur_game->cursor] -= 1;
		if (cur_player->quantity[(int) cur_game->cursor] == 0) cur_player->loot[(int) cur_game->cursor] = 0;
		return SDL_TRUE;
	} else if (loot_type == ROOF && quantity == 1) {
		/* Is there already a roof tile? */
		if (*(*(map->roof+new_y)+new_x) != 0) {
			return SDL_TRUE;
		}
		/* There's no roof, so place it, remove from inventory, and return */
		*(*(map->roof+new_y)+new_x) = cur_item;
		cur_player->quantity[(int) cur_game->cursor] -= 1;
		if (cur_player->quantity[(int) cur_game->cursor] == 0) cur_player->loot[(int) cur_game->cursor] = 0;
		return SDL_TRUE;
	} else if (loot_type == WALL && quantity == 1) {
		/* Is there already an item? */
		if (*(*(map->loot+new_y)+new_x) != 0) {
			return SDL_TRUE;
		}
		/* There's no item, so place a singular wall and return */
		*(*(map->loot+new_y)+new_x) = cur_item;
		*(*(map->quantity+new_y)+new_x) = quantity;
		cur_player->quantity[(int) cur_game->cursor] -= 1;
		if (cur_player->quantity[(int) cur_game->cursor] == 0) cur_player->loot[(int) cur_game->cursor] = 0;
		return SDL_TRUE;		
	} else {
		/* is there an item on the map already? */
		item_on_map = *(*(map->loot+new_y)+new_x);
		/* There's no item on map, just place it and leave */
		if (item_on_map == 0) {
			*(*(map->loot+new_y)+new_x) = cur_item;
			*(*(map->quantity+new_y)+new_x) = quantity;
			cur_player->quantity[(int) cur_game->cursor] = cur_player->quantity[(int) cur_game->cursor] - quantity;
			if (cur_player->quantity[(int) cur_game->cursor] == 0) cur_player->loot[(int) cur_game->cursor] = 0;
			if (loot_type == HOLDER) {
				add_hold(new_x, new_y, NULL, NULL);
			}
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
		unsigned short int drop_how_much;
		if ((int) *(*(map->quantity+new_y)+new_x) + (int) quantity > MAX_STACK) {
			drop_how_much = MAX_STACK - *(*(map->quantity+new_y)+new_x);
		} else {
			drop_how_much = quantity;
		}
		/* Make the change */
		*(*(map->loot+new_y)+new_x) = cur_item;
		*(*(map->quantity+new_y)+new_x) += drop_how_much;
		cur_player->quantity[(int) cur_game->cursor] -= drop_how_much;
	
		/* Any items left? */
		if (cur_player->quantity[(int) cur_game->cursor] == 0) {
			cur_player->loot[(int) cur_game->cursor] = 0;
		}
	}
	return SDL_TRUE;	
}

void
move_cursor(struct game *cur_game, char dir)
{
	char max;
	
	/* Determine max location for cursor */
	switch (cur_game->inventory) {
		case SDL_TRUE:
			max = 39;
			break;
		case SDL_FALSE:
		default:
			max = 7;
			break;
	}
	/* If you have an invisible cursor, set it to the max quickbar slot */
	if (cur_game->cursor > max) {
		if (dir == 1) {
			cur_game->cursor = max;
		} else {
			cur_game->cursor = 0;
		}
		return;
	}
	/* Move cursor */
	cur_game->cursor += dir;
	/* Pac-man if necessary */
	if (cur_game->cursor < 0) {
		cur_game->cursor = max;
	} else if (cur_game->cursor > max) {
		cur_game->cursor = 0;
	}
}

void
swap_item(struct game *cur_game, struct worldmap *map, struct player *cur_player)
{
	char start_pos;
	char white[3] = { 255, 255, 255 };
	char black[3] = { 0, 0, 0 };
	
	/* Are you on an item? */
	if (cur_player->loot[(short int) cur_game->cursor] == 0) { return; }
	start_pos = cur_game->cursor;
		
	/* Wait for user input */
	SDL_Event event;
	SDL_bool finished = SDL_FALSE;
	while (finished == SDL_FALSE) {
		/* Draw screen */
		draw_game(cur_game, map, cur_player);
		draw_rect(cur_game, GAME_X, GAME_Y, GAME_W, GAME_H, SDL_FALSE, white, SDL_FALSE, NULL);
		draw_rect(cur_game, GAME_X + 16, GAME_Y + 16, 28*10 + 2, 18 + 2, SDL_TRUE, black, SDL_TRUE, white);
		draw_sentence(cur_game, GAME_X + 16 + 1, GAME_Y + 16 + 1, "Put where?");
		render_present(cur_game);
		SDL_Delay(10);
		/* Poll for input */
		if (SDL_PollEvent(&event) == 0) continue;
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
				case SDLK_i: /* toggle inventory */
					toggle_inv(cur_game);
					break;
				case SDLK_q: /* move cursor left */
					move_cursor(cur_game, -1);
					break;
				case SDLK_e: /* move cursor right */
					move_cursor(cur_game, 1);
					break;
				case SDLK_f: /* finish the swap */
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_1: /* move to quickbar slot 1 */
					move_cursor(cur_game, 0 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_2: /* move to quickbar slot 1 */
					move_cursor(cur_game, 1 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_3: /* move to quickbar slot 1 */
					move_cursor(cur_game, 2 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_4: /* move to quickbar slot 1 */
					move_cursor(cur_game, 3 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_5: /* move to quickbar slot 1 */
					move_cursor(cur_game, 4 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_6: /* move to quickbar slot 1 */
					move_cursor(cur_game, 5 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_7: /* move to quickbar slot 1 */
					move_cursor(cur_game, 6 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
				case SDLK_8: /* move to quickbar slot 1 */
					move_cursor(cur_game, 7 - cur_game->cursor);
					finished = handle_swap(cur_game, cur_player, start_pos);
					break;
			}
		} else if (event.type == SDL_MOUSEBUTTONDOWN) {
			mouse_click(cur_game, map, cur_player, event.button.x, event.button.y, event.button.button);
			finished = handle_swap(cur_game, cur_player, start_pos);
		}
	}
}

SDL_bool
handle_swap(struct game *cur_game, struct player *cur_player, char start_pos)
{
	unsigned short int tmp_loot;
	unsigned short int tmp_quantity;
	
	/* Not moving item */
	if (start_pos == cur_game->cursor) return SDL_TRUE;
	
	/* Are the items the same? If so, combine if possible */
	if (cur_player->loot[(short int) start_pos] == cur_player->loot[(short int) cur_game->cursor]) {
		if (is_loot_stackable(cur_player->loot[(short int) start_pos]) == SDL_FALSE) {
			return SDL_TRUE;
		}
		if ((int) cur_player->quantity[(short int) start_pos] + (int) cur_player->quantity[(short int) cur_game->cursor] <= MAX_STACK) {
			cur_player->quantity[(short int) cur_game->cursor] += cur_player->quantity[(short int) start_pos];
			cur_player->loot[(short int) start_pos] = 0;
			cur_player->quantity[(short int) start_pos] = 0;
			return SDL_TRUE;
		} else {
			cur_player->quantity[(short int) start_pos] -= MAX_STACK - cur_player->quantity[(short int) cur_game->cursor];
			cur_player->quantity[(short int) cur_game->cursor] = MAX_STACK;
			return SDL_TRUE;
		}
	}
	
	/* Save first item in tmp's */
	tmp_loot = cur_player->loot[(short int) start_pos];
	tmp_quantity = cur_player->quantity[(short int) start_pos];
	/* Move second item into first item slot */
	cur_player->loot[(short int) start_pos] = cur_player->loot[(short int) cur_game->cursor];
	cur_player->quantity[(short int) start_pos] = cur_player->quantity[(short int) cur_game->cursor];
	/* Move tmp into second item slot */
	cur_player->loot[(short int) cur_game->cursor] = tmp_loot;
	cur_player->quantity[(short int) cur_game->cursor] = tmp_quantity;
	/* Finish normally */
	return SDL_TRUE;
}
