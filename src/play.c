#include "loot.h"
#include "main.h"
#include "maps.h"
#include "play.h"

/* Function prototypes */
static void	pacman_player(struct worldmap *map, struct player *cur_player);

void
player_init(struct player *cur_player)
{
	int rows, cols;
	/* Set player position */
	cur_player->x = MAP_COLS/4;
	cur_player->y = MAP_ROWS/4;
	/* Initialize player seen */
	cur_player->seen = malloc(sizeof(*cur_player->seen)*MAP_ROWS);
	for (rows = 0; rows < MAP_ROWS; rows++) {
		*(cur_player->seen+rows) = malloc(sizeof(**cur_player->seen)*MAP_COLS);
		for (cols = 0; cols < MAP_COLS; cols++) {
			*(*(cur_player->seen+rows)+cols) = 1;
		}
	}
	/* Initialize the inventory */
	for (rows = 0; rows < MAX_INV; rows++) {
		cur_player->loot[rows] = 0;
		cur_player->quantity[rows] = 0;
	}
}

void
player_quit(struct player *cur_player)
{
	int rows;
	
	/* Free memory allocated for the player */
	for (rows = 0; rows < MAP_ROWS; rows++) {
		free(*(cur_player->seen+rows));
	}
	free(cur_player->seen);
}

void
move_player(struct worldmap *map, struct player *cur_player, int x, int y)
{
	short int item_num;
	char passable;
	int i;
	/* move player */
	cur_player->x += x;
	cur_player->y += y;
	/* pacman player */
	pacman_player(map, cur_player);
	/* check if tile is impassable */
	passable = is_passable(*(*(map->tile+cur_player->y)+cur_player->x),
	                       *(*(map->biome+cur_player->y)+cur_player->x));
	if (passable == IMPASSABLE) {
		/* unmove player */
		cur_player->x -= x;
		cur_player->y -= y;
		/* un-pacman player */
		pacman_player(map, cur_player);
	}
	/* check for loot */
	item_num = *(*(map->loot+cur_player->y)+cur_player->x);
	if (item_num != 0) {
		/* add to inventory */
		if (is_loot_stackable(item_num) == STACKABLE) {
			for (i = 0; i < 32; i++) {
				if (cur_player->loot[i] == item_num && cur_player->quantity[i] < 255) {
					cur_player->quantity[i] += 1;
					*(*(map->loot+cur_player->y)+cur_player->x) = 0;
					return;
				}
			}
		}
		for (i = 0; i < 32; i++) {
			if (cur_player->loot[i] == 0) {
				cur_player->loot[i] = item_num;
				cur_player->quantity[i] = 1;
				*(*(map->loot+cur_player->y)+cur_player->x) = 0;
				return;
			}
		}	
	}	
}

static void
pacman_player(struct worldmap *map, struct player *cur_player)
{
	if (cur_player->x < 0) {
		cur_player->x = map->col_size - 1;
	} else if (cur_player->x > map->col_size - 1) {
		cur_player->x = 0;
	} else if (cur_player->y < 0) {
		cur_player->y = map->row_size - 1;
	} else if (cur_player->y > map->row_size - 1) {
		cur_player->y = 0;
	}
}
