#include <stdlib.h>
#include "loot.h"
#include "maps.h"

struct loot LOOT[5] = {
	{NULL, 0, UNSTACKABLE, IMPASSABLE},
	{"money", 258, STACKABLE, PASSABLE},
	{"key", 263, UNSTACKABLE, PASSABLE},
	{"necklace", 268, UNSTACKABLE, PASSABLE},
	{"chicken", 271, STACKABLE, PASSABLE}
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
