#ifndef PLAY_H
#define PLAY_H

#include "main.h"
#include "maps.h"

/* Maximum player inventory */
#define MAX_INV 40

/* Main player structure */
struct player {
	int x;
	int y;
	int winpos_x;
	int winpos_y;
	char **seen;
	unsigned short int loot[MAX_INV];
	unsigned char quantity[MAX_INV];
};

extern void	player_init(struct worldmap *map, struct player *cur_player);
extern void	player_quit(struct player *cur_player);
extern void	move_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y);
extern void	toggle_inv(struct game *cur_game);
extern void	random_start(struct worldmap *map, struct player *cur_player);

#endif
