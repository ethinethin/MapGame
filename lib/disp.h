#ifndef DISP_H
#define DISP_H

#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"

extern void	display_init(struct game *cur_game);
extern void	display_quit(struct game *cur_game);
extern void	draw_all(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	load_sprites(struct game *cur_game);
extern void	worldmap(struct game *cur_game, struct worldmap *map, struct player *cur_player);

#endif
