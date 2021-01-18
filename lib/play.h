#ifndef PLAY_H
#define PLAY_H

#include "main.h"
#include "maps.h"

extern void	player_init(struct player *cur_player);
extern void	player_quit(struct player *cur_player);
extern void	move_player(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y);


#endif