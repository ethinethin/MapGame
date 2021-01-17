#include "main.h"
#include "maps.h"

/* Function prototypes */
static void	pacman_player(struct worldmap *map, struct player *cur_player);

void
move_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y)
{
	/* move player */
	cur_player->x += x;
	cur_player->y += y;
	/* pacman player */
	pacman_player(map, cur_player);
	/* check if tile is impassable */
	switch (*(*(map->tile+cur_player->y)+cur_player->x)) {
		case 1: /* grass */
			break;
		case 2: /* mountain */
		case 3: /* water */
		case 4: /* lava */
			/* unmove player */
			cur_player->x -= x;
			cur_player->y -= y;
			break;
		case 5: /* sand */
			break;
		default:
			break;
	}
	/* un-pacman player */
	pacman_player(map, cur_player);
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
