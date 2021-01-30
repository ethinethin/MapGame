#ifndef PLAY_H
#define PLAY_H

//#include "main.h"
#include "maps.h"

/* Maximum player inventory */
#define MAX_INV 32

/* Main player structure */
struct player {
	int x;
	int y;
	char **seen;
	unsigned short int loot[MAX_INV];
	unsigned char quantity[MAX_INV];
};

extern void	player_init(struct player *cur_player);
extern void	player_quit(struct player *cur_player);
extern void	move_player(struct worldmap *map, struct player *cur_player, int x, int y);


#endif
