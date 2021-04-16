#ifndef SAVE_H
#define SAVE_H

#include "main.h"
#include "maps.h"
#include "play.h"

extern void	save_all(struct worldmap *map, struct player *cur_player);
extern void	load_all(struct game *cur_game, struct worldmap *map, struct player *cur_player);

#endif
