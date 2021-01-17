#ifndef DISP_H
#define DISP_H

#include <SDL2/SDL.h>
#include "main.h"
#include "maps.h"

extern void	display_init(struct game *cur_game);
extern void	display_quit(struct game *cur_game);
extern void	draw_rect(struct game *cur_game, unsigned int x, unsigned int y,
			  unsigned int w, unsigned int h, SDL_bool fill,
			  int fill_col[3], SDL_bool border, int bord_col[3]);
extern void	draw_map(struct game *cur_game, struct worldmap *map, struct player *cur_player);
extern void	load_sprites(struct game *cur_game);

#endif
