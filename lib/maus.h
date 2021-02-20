#ifndef MAUS_H
#define MAUS_H

#include "main.h"

extern void	mouse_click(struct game *cur_game, struct worldmap *map, struct player *cur_player, int x, int y, unsigned char button);
extern void	place_items(struct game *cur_game, struct worldmap *map, struct player *cur_player);

#endif
