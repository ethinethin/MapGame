#include <stdlib.h>
#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

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
pickup_item(struct worldmap *map, struct player *cur_player)
{
	short int item_num;
	int i;
	
	/* add to inventory if you can */
	item_num = *(*(map->loot+cur_player->y)+cur_player->x);
	if (is_loot_stackable(item_num) == STACKABLE) {
		for (i = 0; i < MAX_INV; i++) {
			if (cur_player->loot[i] == item_num && cur_player->quantity[i] < 255) {
				cur_player->quantity[i] += 1;
				*(*(map->loot+cur_player->y)+cur_player->x) = 0;
				return;
			}
		}
	}
	for (i = 0; i < MAX_INV; i++) {
		if (cur_player->loot[i] == 0) {
			cur_player->loot[i] = item_num;
			cur_player->quantity[i] = 1;
			*(*(map->loot+cur_player->y)+cur_player->x) = 0;
			return;
		}
	}	
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
