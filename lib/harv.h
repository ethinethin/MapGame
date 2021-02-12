#ifndef HARV_H
#define HARV_H

#include "main.h"
#include "maps.h"
#include "play.h"

extern void		get_harvest_input(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern SDL_bool		harvest_item(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y);
extern SDL_bool		is_harvestable(struct worldmap *map, int x, int y);

#endif
