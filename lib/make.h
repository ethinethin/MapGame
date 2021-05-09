#ifndef MAKE_H
#define MAKE_H

#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"
#include "play.h"

extern void	make_stuff(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	check_recipes(struct player *cur_player);

#endif
